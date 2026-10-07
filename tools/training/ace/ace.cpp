// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <memory>
#include <set>
#include <string_view>
#include <vector>

#include "openzl/cpp/Compressor.hpp"

#include "openzl/cpp/codecs/BruteForce.hpp"
#include "openzl/compress/cgraph.h"
#include "openzl/compress/selectors/selector_brute_force.h"
#include "openzl/zl_reflection.h"
#include "tools/logger/Logger.h"
#include "tools/training/ace/ace.h"
#include "tools/training/ace/ace_compressor.h"
#include "tools/training/ace/ace_compressors.h"
#include "tools/training/ace/automated_compressor_explorer.h"
#include "tools/training/graph_mutation/graph_mutation_utils.h"
#include "tools/training/sample_collection/training_sample_collector.h"
#include "tools/training/utils/mutation.h"
#include "tools/training/utils/pareto_combination.h"
#include "tools/training/utils/serialized_compressor_internal.h"
#include "tools/training/utils/utils.h"

namespace openzl::training {

const std::string ACE_GRAPH_NAME = "zl.ace";

namespace {
using namespace openzl::training::graph_mutation;
using namespace openzl::tools::logger;

/// ACE_CENSUS: one stderr line per generation describing the population, so
/// search behaviour can be compared across variants.
void logCensus(const AutomatedCompressorExplorer& ace, size_t graphIdx)
{
    const auto fitness = ace.fitness();
    const auto rank    = ace.rank();
    const auto pop     = ace.population();
    double srcSize     = 0;
    for (const auto& in : ace.inputs()) {
        srcSize += in.contentSize();
    }
    float best    = std::numeric_limits<float>::infinity();
    size_t noGain = 0;
    for (size_t i = 0; i < pop.size(); ++i) {
        noGain += fitness[i][0] >= 0.95 * srcSize;
        best = std::min(best, fitness[i][0]);
    }
    size_t front0     = 0;
    size_t near       = 0;
    size_t components = 0;
    std::set<uint64_t> sizes;
    for (size_t i = 0; i < pop.size(); ++i) {
        near += fitness[i][0] <= 1.01 * best;
        front0 += rank[i] == 0;
        sizes.insert((uint64_t)fitness[i][0]);
        components += pop[i].numComponents();
    }
    fprintf(stderr,
            "ACE_CENSUS graph=%zu gen=%zu pop=%zu front0=%zu uniq_size=%zu "
            "best=%.0f ratio=%.4f nogain=%zu within1pct=%zu mean_components=%.2f\n",
            graphIdx,
            ace.generation(),
            pop.size(),
            front0,
            sizes.size(),
            best,
            srcSize / best,
            noGain,
            near,
            (double)components / pop.size());
}

class ACEBackendGraphMutation : public BackendGraphMutation {
   public:
    /**
     * @param compressor Replaces the backend graph. Only omitted when
     * @p onlySaveAceState is set.
     * @param aceState Saved onto the backend graph when it is non-null, so that
     * training can later be resumed from it.
     * @param onlySaveAceState Saves @p aceState onto the backend graph but
     * leaves the graph itself alone.
     */
    ACEBackendGraphMutation(
            std::string backendGraph,
            poly::optional<ACECompressor> compressor,
            std::shared_ptr<const std::string> aceState = nullptr,
            bool onlySaveAceState                       = false)
            : BackendGraphMutation(std::move(backendGraph)),
              compressor_(std::move(compressor)),
              aceState_(std::move(aceState)),
              onlySaveAceState_(onlySaveAceState)
    {
        if (onlySaveAceState_) {
            if (!aceState_) {
                throw Exception("ACE state not set when saving the checkpoint");
            }
        } else if (!compressor_.has_value()) {
            throw Exception("No compressor to replace the backend graph with");
        }
    }

    poly::optional<GraphID> newGraph(Compressor& compressor) const override
    {
        if (onlySaveAceState_) {
            return poly::nullopt;
        }
        return compressor_->build(compressor);
    }

    poly::optional<GraphParameters> newGraphParams(
            Compressor& compressor) const override
    {
        if (!aceState_) {
            return {};
        }
        GraphParameters params;
        params.localParams.emplace();
        params.localParams->addCopyParam(
                AutomatedCompressorExplorer::kAceStateParamId,
                aceState_->data(),
                aceState_->size());
        return params;
    }

   private:
    poly::optional<ACECompressor> compressor_;
    std::shared_ptr<const std::string> aceState_;
    bool onlySaveAceState_;
};

/// @returns The names of every ACE backend graph in @p compressor.
std::vector<std::string> findAceGraphs(const Compressor& compressor)
{
    const std::vector<GraphID> graphIDs =
            findAllGraphsWithPrefix(compressor, ACE_GRAPH_NAME);
    std::vector<std::string> graphNames;
    graphNames.reserve(graphIDs.size());
    for (const auto& graphID : graphIDs) {
        graphNames.emplace_back(
                ZL_Compressor_Graph_getName(compressor.get(), graphID));
    }
    return graphNames;
}

/**
 * @returns The ACE state saved on @p backendGraph, or poly::nullopt for the
 * unparameterized graphs which have never been trained.
 */
poly::optional<std::string> getAceState(
        const Compressor& compressor,
        const std::string& backendGraph)
{
    auto backendGraphID = compressor.getGraph(backendGraph);
    if (!backendGraphID.has_value()) {
        throw Exception("Unexpected error: backend graph not found");
    }
    const auto localParams = LocalParams(ZL_Compressor_Graph_getLocalParams(
            compressor.get(), backendGraphID.value()));
    const auto copyParams  = localParams.getCopyParams();
    const auto copyParam   = std::find_if(
            copyParams.begin(), copyParams.end(), [](const auto& param) {
                return param.paramId
                        == AutomatedCompressorExplorer::kAceStateParamId;
            });
    if (copyParam == copyParams.end()) {
        return poly::nullopt;
    }
    return std::string((const char*)copyParam->paramPtr, copyParam->paramSize);
}

/// @returns Every input of every sample in @p samples, referenced in place.
std::vector<Input> flattenSamples(std::vector<MultiInput>& samples)
{
    std::vector<Input> flattened;
    for (auto& sample : samples) {
        for (auto& input : *sample) {
            flattened.push_back(InputRef(input.get()));
        }
    }
    return flattened;
}

/**
 * @returns The compressors on @p ace's Pareto frontier which are able to
 * compress at its format version, best compression ratio first. Only the single
 * best-ratio compressor is returned when @p paretoFrontier is false.
 *
 * Falls back to the store compressor when nothing else can compress.
 */
std::vector<ACECompressor> selectAceCandidates(
        const AutomatedCompressorExplorer& ace,
        bool paretoFrontier)
{
    auto solutions = ace.solution();
    if (solutions.empty()) {
        throw Exception("ACE training failed to find a solution");
    }

    auto inputs = ace.inputs();
    std::vector<ACECompressor> candidates;
    for (auto&& [candidate, _] : solutions) {
        // Drop candidates which cannot compress at the target format version
        if (!candidate
                     .benchmark(
                             inputs,
                             ace.formatVersion(),
                             ace.compressionLevel())
                     .has_value()) {
            continue;
        }
        candidates.push_back(std::move(candidate));
        if (!paretoFrontier) {
            break;
        }
    }
    if (candidates.empty()) {
        Logger::log(
                WARNINGS,
                "No solution found that meets speed constraints: Falling back to store");
        auto store = buildStoreCompressor();
        if (!store.benchmark(
                          inputs, ace.formatVersion(), ace.compressionLevel())
                     .has_value()) {
            throw Exception(
                    "Store compressor failed to compress at the target format version");
        }
        candidates.push_back(std::move(store));
    }
    return candidates;
}

/// @returns The ACE run configuration described by @p trainParams.
/// @param maxTime Time budget for this run.
AutomatedCompressorExplorer::Parameters aceParameters(
        const TrainParams& trainParams,
        uint32_t formatVersion,
        int compressionLevel,
        poly::optional<std::chrono::seconds> maxTime)
{
    AutomatedCompressorExplorer::Parameters params{
        .numThreads = trainParams.threads.value_or(
                std::thread::hardware_concurrency() / 2),
    };
    params.maxTime          = maxTime;
    params.formatVersion    = formatVersion;
    params.compressionLevel = compressionLevel;
    if (const char* g = std::getenv("ACE_MAX_GENERATIONS")) {
        params.maxGenerations = std::strtoul(g, nullptr, 10);
    }
    if (const char* s = std::getenv("ACE_SEED")) {
        params.seed = std::strtoul(s, nullptr, 10);
    }
    return params;
}

/// Runs @p ace to completion, reporting its progress.
void runTraining(
        AutomatedCompressorExplorer& ace,
        size_t graphIdx,
        size_t numGraphs)
{
    const bool census = std::getenv("ACE_CENSUS") != nullptr;
    for (;;) {
        if (census && ace.generation() > 0) {
            logCensus(ace, graphIdx);
        }
        Logger::logProgress(
                INFO,
                ace.progress(),
                "Training ACE graph %zu / %zu: ACE progress",
                graphIdx,
                numGraphs);
        if (ace.finished()) {
            break;
        }
        ace.step();
    }
    Logger::finalizeProgress(INFO);
}

/**
 * @returns A mutation replacing @p backendGraph for each compressor on @p ace's
 * Pareto frontier.
 *
 * @param aceState Saved onto the graph by every mutation when non-null.
 */
std::vector<std::unique_ptr<BackendGraphMutation>> makeAceMutations(
        const std::string& backendGraph,
        const AutomatedCompressorExplorer& ace,
        std::shared_ptr<const std::string> aceState,
        bool paretoFrontier)
{
    auto candidates = selectAceCandidates(ace, paretoFrontier);
    std::vector<std::unique_ptr<BackendGraphMutation>> mutations;
    mutations.reserve(candidates.size());
    for (auto& candidate : candidates) {
        mutations.push_back(
                std::make_unique<ACEBackendGraphMutation>(
                        backendGraph, std::move(candidate), aceState));
    }
    return mutations;
}

/**
 * Replaces the backend graph with a brute-force selector over @p picks, so
 * each stream uses its smallest pick at compression time.
 *
 * The selector reads its int params from the graph it runs as. After
 * ZL_Compressor_overrideBaseGraph() that is the zl.ace wrapper graph, and its
 * local params are replaced by newGraphParams(), so @p intParams are attached
 * to the wrapper together with the ACE state, not to the brute force graph.
 */
class ACEBruteMutation : public BackendGraphMutation {
   public:
    ACEBruteMutation(
            std::string backendGraph,
            std::vector<ACECompressor> picks,
            std::vector<ZL_IntParam> intParams,
            std::shared_ptr<const std::string> aceState)
            : BackendGraphMutation(std::move(backendGraph)),
              picks_(std::move(picks)),
              intParams_(std::move(intParams)),
              aceState_(std::move(aceState))
    {
    }

    poly::optional<GraphID> newGraph(Compressor& compressor) const override
    {
        std::vector<GraphID> successors;
        successors.reserve(picks_.size());
        for (const auto& pick : picks_) {
            successors.push_back(pick.build(compressor));
        }
        return graphs::BruteForce(std::move(successors))
                .parameterize(compressor);
    }

    poly::optional<GraphParameters> newGraphParams(
            Compressor& compressor) const override
    {
        if (intParams_.empty() && !aceState_) {
            return {};
        }
        GraphParameters params;
        params.localParams.emplace();
        for (const auto& p : intParams_) {
            params.localParams->addIntParam(p);
        }
        if (aceState_) {
            params.localParams->addCopyParam(
                    AutomatedCompressorExplorer::kAceStateParamId,
                    aceState_->data(),
                    aceState_->size());
        }
        return params;
    }

   private:
    std::vector<ACECompressor> picks_;
    std::vector<ZL_IntParam> intParams_;
    std::shared_ptr<const std::string> aceState_;
};

/**
 * ACE_BRUTE_K=K: greedily picks up to K candidates from @p ace that complement
 * each other on its inputs (each pick most reduces the sum over inputs of the
 * best size so far). With ACE_BRUTE_TOL=t every gene whose size is within
 * (1 + t) of the smallest is a candidate, not only front 0.
 *
 * @param rates Receives each pick's decode cost in ns per KiB of input when
 * ACE_BRUTE_LAMBDA is set.
 */
std::vector<ACECompressor> pickComplementary(
        const AutomatedCompressorExplorer& ace,
        const std::string& backendGraph,
        size_t maxPicks,
        std::vector<int>& rates)
{
    auto solutions = ace.solution();
    if (const char* tol = std::getenv("ACE_BRUTE_TOL")) {
        const double t = std::strtod(tol, nullptr);
        float best     = std::numeric_limits<float>::infinity();
        for (const auto& f : ace.fitness()) {
            best = std::min(best, f[0]);
        }
        solutions.clear();
        for (size_t i = 0; i < ace.population().size(); ++i) {
            if (ace.fitness()[i][0] <= best * (1 + t)) {
                solutions.emplace_back(ace.population()[i], ace.fitness()[i]);
            }
        }
    }
    const auto inputs = ace.inputs();
    const size_t n    = inputs.size();
    auto benchmarkOne = [&](const ACECompressor& c, size_t i) {
        return c.benchmark(
                inputs.subspan(i, 1), ace.formatVersion(), ace.compressionLevel());
    };
    std::vector<std::vector<double>> sizes;
    std::vector<double> dtime;
    std::vector<ACECompressor> cands;
    for (auto& [cand, _] : solutions) {
        std::vector<double> s(n);
        double dns = 0;
        bool ok    = true;
        for (size_t i = 0; i < n && ok; ++i) {
            auto r = benchmarkOne(cand, i);
            ok     = r.has_value();
            if (ok) {
                s[i] = (double)r->compressedSize;
                dns += (double)r->decompressionTime.count();
            }
        }
        if (ok) {
            sizes.push_back(std::move(s));
            dtime.push_back(dns);
            cands.push_back(cand);
        }
    }
    std::vector<ACECompressor> picked;
    std::vector<double> pickedDtime;
    // Each extra pick costs a full trial compression per stream, so it must
    // save at least ACE_BRUTE_MIN_GAIN (default 0.2%) on the training inputs.
    const char* minGainEnv = std::getenv("ACE_BRUTE_MIN_GAIN");
    const double minGain = minGainEnv ? std::strtod(minGainEnv, nullptr) : 0.002;
    std::vector<double> best(n, std::numeric_limits<double>::infinity());
    std::vector<bool> used(cands.size(), false);
    double total = std::numeric_limits<double>::infinity();
    while (picked.size() < maxPicks) {
        size_t bestIdx = cands.size();
        double bestTot = total;
        for (size_t c = 0; c < cands.size(); ++c) {
            if (used[c]) {
                continue;
            }
            double t = 0;
            for (size_t i = 0; i < n; ++i) {
                t += std::min(best[i], sizes[c][i]);
            }
            if (t < bestTot) {
                bestTot = t;
                bestIdx = c;
            }
        }
        if (bestIdx == cands.size()
            || (!picked.empty() && bestTot > (1 - minGain) * total)) {
            Logger::log(
                    INFO,
                    "ACE_BRUTE ",
                    backendGraph,
                    " stop: front ",
                    cands.size(),
                    " best next ",
                    (size_t)bestTot,
                    " vs ",
                    (size_t)total);
            break;
        }
        used[bestIdx] = true;
        for (size_t i = 0; i < n; ++i) {
            best[i] = std::min(best[i], sizes[bestIdx][i]);
        }
        Logger::log(
                INFO,
                "ACE_BRUTE ",
                backendGraph,
                " front ",
                cands.size(),
                " inputs ",
                n,
                " pick ",
                picked.size(),
                ": train bytes ",
                (size_t)total,
                " -> ",
                (size_t)bestTot);
        total = bestTot;
        picked.push_back(cands[bestIdx]);
        pickedDtime.push_back(dtime[bestIdx]);
    }
    // ACE_BRUTE_SLACK: the selector takes the first successor within the slack
    // of the smallest, so list the fastest-decoding successors first.
    if (std::getenv("ACE_BRUTE_SLACK") != nullptr) {
        std::vector<size_t> order(picked.size());
        for (size_t k = 0; k < order.size(); ++k) {
            order[k] = k;
        }
        std::sort(order.begin(), order.end(), [&](size_t x, size_t y) {
            return pickedDtime[x] < pickedDtime[y];
        });
        std::vector<ACECompressor> sorted;
        for (auto k : order) {
            sorted.push_back(picked[k]);
        }
        picked = std::move(sorted);
    }
    // ACE_BRUTE_LAMBDA: the selector needs each pick's decode cost per input
    // byte. Training-time decode timings are noisy under load, so take the min
    // of 3 timings per input.
    if (std::getenv("ACE_BRUTE_LAMBDA") != nullptr) {
        double bytes = 0;
        for (size_t i = 0; i < n; ++i) {
            bytes += (double)inputs[i].contentSize();
        }
        for (const auto& p : picked) {
            double ns = 0;
            for (size_t i = 0; i < n; ++i) {
                double m = std::numeric_limits<double>::infinity();
                for (int rep = 0; rep < 3; ++rep) {
                    auto r = benchmarkOne(p, i);
                    if (r.has_value()) {
                        m = std::min(m, (double)r->decompressionTime.count());
                    }
                }
                ns += std::isfinite(m) ? m : 0;
            }
            rates.push_back((int)std::lround(ns / bytes * 1024.0));
        }
    }
    return picked;
}

/**
 * @returns A single mutation putting the ACE_BRUTE_K complementary picks of
 * @p ace under a brute-force selector, or an empty vector when there is no
 * pick.
 */
std::vector<std::unique_ptr<BackendGraphMutation>> makeBruteMutations(
        const std::string& backendGraph,
        const AutomatedCompressorExplorer& ace,
        std::shared_ptr<const std::string> aceState,
        size_t bruteK)
{
    std::vector<int> rates;
    auto picks = pickComplementary(ace, backendGraph, bruteK, rates);
    std::vector<std::unique_ptr<BackendGraphMutation>> mutations;
    if (picks.empty()) {
        return mutations;
    }
    if (picks.size() == 1) {
        mutations.push_back(
                std::make_unique<ACEBackendGraphMutation>(
                        backendGraph, std::move(picks[0]), aceState));
        return mutations;
    }
    std::vector<ZL_IntParam> intParams;
    if (const char* s = std::getenv("ACE_BRUTE_SLACK")) {
        intParams.push_back(
                { ZL_BRUTE_FORCE_SLACK_PARAM,
                  (int)std::strtol(s, nullptr, 10) });
    }
    if (const char* l = std::getenv("ACE_BRUTE_LAMBDA")) {
        intParams.push_back(
                { ZL_BRUTE_FORCE_LAMBDA_PARAM,
                  (int)std::strtol(l, nullptr, 10) });
        for (size_t k = 0; k < rates.size(); ++k) {
            intParams.push_back(
                    { ZL_BRUTE_FORCE_RATE_PARAM_BASE + (int)k, rates[k] });
        }
    }
    mutations.push_back(
            std::make_unique<ACEBruteMutation>(
                    backendGraph,
                    std::move(picks),
                    std::move(intParams),
                    std::move(aceState)));
    return mutations;
}

/// @returns The compressor produced by applying @p mutations in order.
Compressor applyMutations(
        const std::function<Compressor()>& makeCompressor,
        const std::vector<std::unique_ptr<BackendGraphMutation>>& mutations)
{
    auto compressor = makeCompressor();
    for (const auto& mutation : mutations) {
        mutation->mutate(compressor);
    }
    return compressor;
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

    const auto autoBackendGraphs = findAceGraphs(compressor);

    Logger::log(
            VERBOSE1,
            "Found ",
            autoBackendGraphs.size(),
            " ACE graphs in compressor");

    auto samples =
            collectInputStreamsForGraphs(inputs, autoBackendGraphs, cctx);
    const auto formatVersion = static_cast<uint32_t>(
            compressor.getParameter(CParam::FormatVersion));
    const int compressionLevel =
            compressor.getParameter(CParam::CompressionLevel);

    MergedParetoFrontier::BackendGraphMutationsMap candidates;
    std::vector<std::unique_ptr<BackendGraphMutation>> checkPointMutations;

    // ACE_BRUTE_K=K (K > 1): each graph becomes a brute-force selector over
    // up to K complementary picks. Only applies without --pareto-frontier.
    const char* bruteEnv = std::getenv("ACE_BRUTE_K");
    const size_t bruteK  = bruteEnv ? std::strtoul(bruteEnv, nullptr, 10) : 1;
    const bool brute     = bruteK > 1 && !trainParams.paretoFrontier;
    // ACE_BRUTE_K1_OUT=path: also writes the plain smallest-candidate
    // compressor from the same search, so selection is measured without
    // search noise.
    const char* k1Out = trainParams.paretoFrontier
            ? nullptr
            : std::getenv("ACE_BRUTE_K1_OUT");
    MergedParetoFrontier::BackendGraphMutationsMap k1Candidates;

    // One budget for the whole phase: each graph builds its own explorer, so
    // an unshared deadline is spent once per graph.
    poly::optional<std::chrono::steady_clock::time_point> deadline;
    if (trainParams.maxTimeSecs.has_value()) {
        deadline = std::chrono::steady_clock::now()
                + std::chrono::seconds(*trainParams.maxTimeSecs);
    }

    size_t graphIdx        = 0;
    const size_t numGraphs = autoBackendGraphs.size();
    for (const auto& backendGraph : autoBackendGraphs) {
        ++graphIdx;
        // Skip graphs with no training samples (e.g., optional fields that
        // aren't present in the training data). These will use default
        // compression.
        auto flattened = flattenSamples(samples[backendGraph]);
        if (flattened.empty()) {
            Logger::log(
                    VERBOSE1,
                    "Skipping ACE graph ",
                    graphIdx,
                    " / ",
                    numGraphs,
                    " (",
                    backendGraph,
                    "): no training samples");
            continue;
        }

        // Graphs which have already been trained resume from their saved
        // population, the rest are trained from scratch.
        const auto savedAceState = getAceState(compressor, backendGraph);
        if (skipTraining_ && !savedAceState.has_value()) {
            Logger::log(
                    VERBOSE1,
                    "Skipping ACE graph ",
                    graphIdx,
                    " / ",
                    numGraphs,
                    " (",
                    backendGraph,
                    "): no ACE state to select from");
            continue;
        }

        // An equal share of what is left, so early graphs cannot drain the
        // budget. Unused time rolls forward to the graphs after this one.
        poly::optional<std::chrono::seconds> maxTime;
        if (deadline.has_value() && !skipTraining_) {
            // Rounded up: seconds granularity truncates a short remainder away.
            const auto remaining = std::chrono::ceil<std::chrono::seconds>(
                    *deadline - std::chrono::steady_clock::now());
            if (remaining <= std::chrono::seconds(0)) {
                Logger::log_c(
                        INFO,
                        "Stopping ACE training early at graph %zu / %zu. Exceeded max time of %zu s.",
                        graphIdx,
                        numGraphs,
                        *trainParams.maxTimeSecs);
                break;
            }
            const auto graphsLeft = numGraphs - graphIdx + 1;
            maxTime               = std::chrono::seconds(
                    std::max<int64_t>(
                            1, remaining.count() / int64_t(graphsLeft)));
        }

        AutomatedCompressorExplorer ace(
                flattened,
                aceParameters(
                        trainParams, formatVersion, compressionLevel, maxTime));
        if (savedAceState.has_value()) {
            ace.loadPopulation(*savedAceState);
        }
        if (!skipTraining_) {
            runTraining(ace, graphIdx, numGraphs);
        }

        // Every candidate for a graph shares that graph's ACE state
        auto aceState =
                std::make_shared<const std::string>(ace.savePopulation());
        checkPointMutations.push_back(
                std::make_unique<ACEBackendGraphMutation>(
                        backendGraph,
                        poly::nullopt,
                        aceState,
                        /* onlySaveAceState */ true));
        auto savedState = trainParams.saveAceState ? aceState : nullptr;
        std::vector<std::unique_ptr<BackendGraphMutation>> mutations;
        if (brute) {
            mutations = makeBruteMutations(backendGraph, ace, savedState, bruteK);
        }
        if (mutations.empty()) {
            mutations = makeAceMutations(
                    backendGraph, ace, savedState, trainParams.paretoFrontier);
        }
        if (k1Out != nullptr) {
            k1Candidates.emplace(
                    backendGraph,
                    makeAceMutations(
                            backendGraph, ace, savedState, /* paretoFrontier */ false));
        }
        candidates.emplace(backendGraph, std::move(mutations));
    }

    checkPoint_.emplace(
            applyMutations(makeCompressor, checkPointMutations).serialize());

    if (k1Out != nullptr) {
        MergedParetoFrontier k1(
                makeCompressor, std::move(k1Candidates), inputs, trainParams);
        std::ofstream(k1Out, std::ios::binary) << *k1.serializedAt(0);
    }

    MergedParetoFrontier frontier(
            makeCompressor, std::move(candidates), inputs, trainParams);
    return frontier.paretoFrontier();
}

} // namespace openzl::training
