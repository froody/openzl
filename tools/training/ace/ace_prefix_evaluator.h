// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

#include "openzl/cpp/poly/Optional.hpp"
#include "openzl/cpp/poly/Span.hpp"
#include "tools/training/ace/ace_compressor.h"
#include "tools/training/utils/thread_pool.h"

namespace openzl {
namespace training {

/**
 * @brief Prefix-tree evaluator for ACE population candidates.
 *
 * Deduplicates prefix subtrees across population candidate compressors by
 * computing intermediate transform streams once and evaluating child branches
 * on the retained intermediate streams using Depth-First Search (DFS).
 */
class ACEPrefixEvaluator {
   public:
    struct PrefixTrieNode {
        uint64_t componentHash{ 0 };
        poly::optional<ACECompressor> component{};

        // Indices of candidate genes that pass through or terminate at this node
        std::vector<size_t> geneIndices;

        // Child branches keyed by child component hash
        std::unordered_map<uint64_t, std::unique_ptr<PrefixTrieNode>> children;

        // Suffix ACECompressor starting from this node to leaf for each candidate index
        std::unordered_map<size_t, ACECompressor> candidateSuffixes;
    };

    /**
     * @brief Evaluates all candidates in the population using prefix-tree work sharing.
     */
    static std::vector<poly::optional<ACECompressionResult>> evaluatePopulation(
            poly::span<const ACECompressor> candidates,
            poly::span<const Input> inputs,
            ThreadPool& threadPool,
            poly::span<const ACECompressor> history = {});

    /**
     * @brief Builds a prefix trie from a population of candidate compressors.
     */
    static std::unique_ptr<PrefixTrieNode> buildPrefixTrie(
            poly::span<const ACECompressor> candidates);
};

} // namespace training
} // namespace openzl
