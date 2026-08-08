// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <thread>

#include "openzl/compress/cctx.h"
#include "openzl/cpp/Exception.hpp"
#include "openzl/zl_compress.h"

#include "tools/logger/Logger.h"
#include "tools/training/sample_collection/introspection_hooks.h"
#include "tools/training/sample_collection/training_sample_collector.h"
#include "tools/training/utils/thread_pool.h"

namespace openzl::training {

using namespace tools::logger;

std::map<std::string, std::vector<MultiInput>> collectInputStreams(
        const std::vector<MultiInput>& inputs,
        const std::vector<std::string>& graphNames,
        const std::vector<std::string>& nodeNames,
        CCtx& cctx)
{
    if (graphNames.empty() && nodeNames.empty()) {
        return {};
    }

    // double-check for duplicates
    {
        std::unordered_set<std::string> names;
        for (const auto& graphName : graphNames) {
            if (names.find(graphName) != names.end()) {
                throw Exception("Duplicate graph name: " + graphName);
            }
            names.insert(graphName);
        }
        for (const auto& nodeName : nodeNames) {
            if (names.find(nodeName) != names.end()) {
                throw Exception("Duplicate node name: " + nodeName);
            }
            names.insert(nodeName);
        }
    }

    Logger::log_c(
            VERBOSE1,
            "Collecting input streams for %zu graphs and %zu nodes",
            graphNames.size(),
            nodeNames.size());

    std::map<std::string, std::vector<MultiInput>> result;

    if (inputs.size() <= 1) {
        for (const auto& mi : inputs) {
            SampleCollectionHook hooks(graphNames, nodeNames);
            openzl::unwrap(
                    ZL_CCtx_attachIntrospectionHooks(
                            cctx.get(), hooks.getRawHooks()),
                    "Failed to attach introspection hooks",
                    cctx.get());

            cctx.compress(*mi);

            for (auto& [name, samples] : hooks.getInputs()) {
                for (auto& sample : samples) {
                    result[name].push_back(std::move(sample));
                }
            }

            openzl::unwrap(
                    ZL_CCtx_detachAllIntrospectionHooks(cctx.get()),
                    "Failed to detach introspection hooks",
                    cctx.get());
        }
    } else {
        size_t numThreads = std::min<size_t>(
                inputs.size(), std::thread::hardware_concurrency());
        if (numThreads < 1) {
            numThreads = 1;
        }
        ThreadPool pool(numThreads);

        std::vector<std::future<std::map<std::string, std::vector<MultiInput>>>>
                futures;
        futures.reserve(inputs.size());

        for (size_t i = 0; i < inputs.size(); ++i) {
            futures.emplace_back(pool.run([&cctx,
                                           &graphNames,
                                           &nodeNames,
                                           &inputs,
                                           i]() {
                CCtx threadCctx = CCtx::createDerived(cctx);
                SampleCollectionHook hooks(graphNames, nodeNames);
                openzl::unwrap(
                        ZL_CCtx_attachIntrospectionHooks(
                                threadCctx.get(), hooks.getRawHooks()),
                        "Failed to attach introspection hooks",
                        threadCctx.get());

                threadCctx.compress(*inputs[i]);

                auto captured = hooks.getInputs();

                openzl::unwrap(
                        ZL_CCtx_detachAllIntrospectionHooks(threadCctx.get()),
                        "Failed to detach introspection hooks",
                        threadCctx.get());

                return captured;
            }));
        }

        for (auto& future : futures) {
            auto sampleMap = future.get();
            for (auto& [name, samples] : sampleMap) {
                for (auto& sample : samples) {
                    result[name].push_back(std::move(sample));
                }
            }
        }
    }

    return result;
}

std::vector<MultiInput> collectInputStreamsForGraph(
        const std::vector<MultiInput>& inputs,
        const std::string& untrainedGraphName,
        CCtx& cctx)
{
    auto map = collectInputStreams(inputs, { untrainedGraphName }, {}, cctx);
    return std::move(map[untrainedGraphName]);
}

std::map<std::string, std::vector<MultiInput>> collectInputStreamsForGraphs(
        const std::vector<MultiInput>& inputs,
        const std::vector<std::string>& untrainedGraphNames,
        CCtx& cctx)
{
    return collectInputStreams(inputs, untrainedGraphNames, {}, cctx);
}

std::vector<MultiInput> collectInputStreamsForNode(
        const std::vector<MultiInput>& inputs,
        const std::string& nodeName,
        CCtx& cctx)
{
    auto map = collectInputStreams(inputs, {}, { nodeName }, cctx);
    return std::move(map[nodeName]);
}

} // namespace openzl::training
