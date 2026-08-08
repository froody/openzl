// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <memory>
#include <string_view>
#include <vector>

#include "openzl/cpp/Compressor.hpp"

#include "openzl/compress/cgraph.h"
#include "tools/logger/Logger.h"
#include "tools/training/ace/ace.h"
#include "tools/training/ace/ace_combination.h"
#include "tools/training/ace/ace_compressor.h"
#include "tools/training/ace/automated_compressor_explorer.h"
#include "tools/training/graph_mutation/graph_mutation_utils.h"
#include "tools/training/sample_collection/training_sample_collector.h"
#include "tools/training/utils/serialized_compressor_internal.h"
#include "tools/training/utils/utils.h"

namespace openzl::training {

const std::string ACE_GRAPH_NAME = "zl.ace";

namespace {
using namespace openzl::training::graph_mutation;
using namespace openzl::tools::logger;

/**
 * @returns The Pareto-optimal set of compressors for @p samples.
 */
std::string trainBackend(
        std::vector<MultiInput>& samples,
        const TrainParams& trainParams,
        size_t graphIdx,
        size_t numGraphs)
{
    if (samples.empty()) {
        throw Exception(
                "There cannot be no samples for a backend graph to be trained.");
    }
    auto flattened = std::vector<Input>();
    for (auto& sample : samples) {
        for (auto& input : *sample) {
            flattened.push_back(InputRef(input.get()));
        }
    }
    poly::optional<std::chrono::seconds> maxTime;
    if (trainParams.maxTimeSecs.has_value()) {
        maxTime = std::chrono::seconds(trainParams.maxTimeSecs.value());
    } else {
        maxTime = poly::nullopt;
    }
    AutomatedCompressorExplorer::Parameters params{
        .numThreads = trainParams.threads.has_value()
                ? trainParams.threads.value()
                : std::thread::hardware_concurrency() / 2,
    };
    params.maxTime = maxTime;
    if (trainParams.populationSize.has_value()) {
        params.populationSize = trainParams.populationSize.value();
    }
    AutomatedCompressorExplorer ace(flattened, params);
    auto startTime = std::chrono::steady_clock::now();

    double smoothedRemainingSecs = -1.0;

    auto updateProgress = [&](float progress) {
        auto const now = std::chrono::steady_clock::now();
        auto const elapsedSecs =
                std::chrono::duration_cast<std::chrono::seconds>(now - startTime)
                        .count();

        std::string etaStr = "";
        if (elapsedSecs >= 3 && progress > 0.001f) {
            double const totalEstSecs =
                    static_cast<double>(elapsedSecs) / static_cast<double>(progress);
            double const rawRemainingSecs = totalEstSecs - static_cast<double>(elapsedSecs);

            if (rawRemainingSecs > 0) {
                if (smoothedRemainingSecs < 0.0) {
                    smoothedRemainingSecs = rawRemainingSecs;
                } else {
                    // Exponential Moving Average with alpha = 0.05 for high stability
                    smoothedRemainingSecs = 0.05 * rawRemainingSecs + 0.95 * smoothedRemainingSecs;
                }

                int const remMin = static_cast<int>(smoothedRemainingSecs) / 60;
                int const remSec = static_cast<int>(smoothedRemainingSecs) % 60;
                if (remMin > 0) {
                    etaStr = " (ETA: " + std::to_string(remMin) + "m "
                            + std::to_string(remSec) + "s)";
                } else {
                    etaStr = " (ETA: " + std::to_string(remSec) + "s)";
                }
            }
        }

        Logger::logProgress(
                INFO,
                progress,
                "Training ACE graph %zu / %zu%s: ACE progress",
                graphIdx,
                numGraphs,
                etaStr.c_str());
    };

    ace.setProgressCallback(updateProgress);

    for (;;) {
        updateProgress(static_cast<float>(ace.progress()));
        if (ace.finished()) {
            break;
        }
        ace.step();
    }
    Logger::finalizeProgress(INFO);
    return ace.savePopulation();
}

} // namespace

std::vector<SerializedCompressorInternal> ACETrainer::train(
        const std::vector<MultiInput>& inputs,
        std::string_view serializedCompressorInput,
        const TrainParams& trainParams)
{
    auto makeCompressor = [&serializedCompressorInput, &trainParams] {
        return std::move(
                *trainParams.compressorGenFunc(serializedCompressorInput, ""));
    };
    auto compressor = makeCompressor();
    auto cctx       = refCCtxForTraining(compressor);

    const std::vector<GraphID> autoBackendGraphIDs =
            findAllGraphsWithPrefix(compressor, ACE_GRAPH_NAME);
    std::vector<std::string> autoBackendGraphs;
    autoBackendGraphs.reserve(autoBackendGraphIDs.size());
    for (const auto& graphID : autoBackendGraphIDs) {
        autoBackendGraphs.emplace_back(
                ZL_Compressor_Graph_getName(compressor.get(), graphID));
    }

    Logger::log(
            VERBOSE1,
            "Found ",
            autoBackendGraphs.size(),
            " ACE graphs in compressor");

    auto samples =
            collectInputStreamsForGraphs(inputs, autoBackendGraphs, cctx);

    std::unordered_map<
            std::string,
            std::vector<std::pair<ACECompressor, ACECompressionResult>>>
            candidates;

    std::unordered_map<std::string, LocalParams> paramsToReplace;

    struct GraphTask {
        size_t graphIdx;
        std::string backendGraph;
    };
    std::vector<GraphTask> activeGraphTasks;
    size_t totalGraphIdx   = 0;
    const size_t numGraphs = autoBackendGraphs.size();
    for (const auto& backendGraph : autoBackendGraphs) {
        ++totalGraphIdx;
        // Skip graphs with no training samples (e.g., optional fields that
        // aren't present in the training data). These will use default
        // compression.
        if (samples[backendGraph].empty()) {
            Logger::log(
                    VERBOSE1,
                    "Skipping ACE graph ",
                    totalGraphIdx,
                    " / ",
                    numGraphs,
                    " (",
                    backendGraph,
                    "): no training samples");
            continue;
        }
        activeGraphTasks.push_back({ totalGraphIdx, backendGraph });
    }

    if (activeGraphTasks.empty()) {
        checkPoint_.emplace(compressor.serialize());
        return getCombinedCompressors(inputs, *checkPoint_, trainParams);
    }

    size_t const totalRequestedThreads = trainParams.threads.value_or(
            std::thread::hardware_concurrency());
    size_t const numActiveGraphs = activeGraphTasks.size();

    // Divide total requested threads across concurrent graphs
    size_t const threadsPerGraph =
            std::max<size_t>(1, totalRequestedThreads / numActiveGraphs);

    std::vector<std::string> aceStates(numActiveGraphs);

    if (numActiveGraphs == 1) {
        aceStates[0] = trainBackend(
                samples[activeGraphTasks[0].backendGraph],
                trainParams,
                activeGraphTasks[0].graphIdx,
                numGraphs);
    } else {
        Logger::log(
                INFO,
                "Training ",
                numActiveGraphs,
                " ACE graphs in parallel (",
                threadsPerGraph,
                " threads/graph, ",
                totalRequestedThreads,
                " total threads)...");

        ThreadPool graphPool(numActiveGraphs);
        std::vector<std::future<std::string>> futures;
        futures.reserve(numActiveGraphs);

        for (size_t i = 0; i < numActiveGraphs; ++i) {
            TrainParams graphParams = trainParams;
            graphParams.threads     = threadsPerGraph;

            auto const& task = activeGraphTasks[i];
            futures.emplace_back(graphPool.run(
                    [&samples, graphParams, task, numGraphs]() {
                        return trainBackend(
                                samples[task.backendGraph],
                                graphParams,
                                task.graphIdx,
                                numGraphs);
                    }));
        }

        for (size_t i = 0; i < numActiveGraphs; ++i) {
            aceStates[i] = futures[i].get();
        }
    }

    for (size_t i = 0; i < activeGraphTasks.size(); ++i) {
        const auto& backendGraph = activeGraphTasks[i].backendGraph;
        const auto& aceState     = aceStates[i];

        auto localParams = LocalParams();
        localParams.addCopyParam(
                AutomatedCompressorExplorer::kAceStateParamId,
                aceState.data(),
                aceState.size());
        auto backendGraphID =
                ZL_Compressor_getGraph(compressor.get(), backendGraph.c_str());
        auto graphs = ZL_Compressor_Graph_getCustomGraphs(
                compressor.get(), backendGraphID);
        auto nodes = ZL_Compressor_Graph_getCustomNodes(
                compressor.get(), backendGraphID);
        auto gp = ZL_GraphParameters{ .customGraphs   = graphs.graphids,
                                      .nbCustomGraphs = graphs.nbGraphIDs,
                                      .customNodes    = nodes.nodeids,
                                      .nbCustomNodes  = nodes.nbNodeIDs,
                                      .localParams    = localParams.get() };
        compressor.unwrap(
                ZL_Compressor_overrideGraphParams(
                        compressor.get(), backendGraphID, &gp),
                "Graph replacement failed");
    }
    checkPoint_.emplace(compressor.serialize());
    return getCombinedCompressors(inputs, *checkPoint_, trainParams);
}
} // namespace openzl::training
