// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "tools/training/ace/ace_prefix_evaluator.h"
#include "tools/training/ace/ace_compressors.h"
#include "tools/logger/Logger.h"

namespace openzl {
namespace training {

namespace {

void extractLinearPipeline(
        const ACECompressor& comp,
        std::vector<ACECompressor>& path)
{
    path.push_back(comp);
    if (comp.isNode()) {
        const auto& nodeComp = comp.asNode();
        if (nodeComp.successors.size() == 1) {
            extractLinearPipeline(*nodeComp.successors[0], path);
        }
    }
}

} // namespace

std::unique_ptr<ACEPrefixEvaluator::PrefixTrieNode>
ACEPrefixEvaluator::buildPrefixTrie(
        poly::span<const ACECompressor> candidates)
{
    auto root = std::make_unique<PrefixTrieNode>();

    for (size_t i = 0; i < candidates.size(); ++i) {
        const auto& candidate = candidates[i];
        root->geneIndices.push_back(i);

        std::vector<ACECompressor> steps;
        extractLinearPipeline(candidate, steps);

        PrefixTrieNode* curr = root.get();
        for (size_t s = 0; s < steps.size(); ++s) {
            const auto& step = steps[s];
            uint64_t h = step.hash();

            auto it = curr->children.find(h);
            if (it == curr->children.end()) {
                auto child = std::make_unique<PrefixTrieNode>();
                child->componentHash = h;
                child->component = step;
                it = curr->children.emplace(h, std::move(child)).first;
            }

            curr = it->second.get();
            curr->geneIndices.push_back(i);

            if (s + 1 < steps.size()) {
                curr->candidateSuffixes.emplace(i, steps[s + 1]);
            } else {
                curr->candidateSuffixes.emplace(i, step);
            }
        }
    }
    return root;
}

std::vector<poly::optional<ACECompressionResult>>
ACEPrefixEvaluator::evaluatePopulation(
        poly::span<const ACECompressor> candidates,
        poly::span<const Input> inputs,
        ThreadPool& threadPool)
{
    std::vector<poly::optional<ACECompressionResult>> results(candidates.size(), poly::nullopt);

    // Build the prefix trie from the current candidate population
    auto root = buildPrefixTrie(candidates);

    // Evaluate candidates in parallel across threadPool
    std::vector<std::future<poly::optional<ACECompressionResult>>> futures;
    futures.reserve(candidates.size());

    for (size_t i = 0; i < candidates.size(); ++i) {
        futures.emplace_back(threadPool.run([&candidate = candidates[i], &inputs] {
            return candidate.benchmark(inputs);
        }));
    }

    for (size_t i = 0; i < candidates.size(); ++i) {
        results[i] = futures[i].get();
    }

    return results;
}

} // namespace training
} // namespace openzl
