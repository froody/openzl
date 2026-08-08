// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "tools/training/utils/utils.h"

namespace openzl::training {

enum class PreconditioningCandidate {
    None,
    Morton,
    Range,
    QuaRs,
    BitPlane,
    IWT,
    Delta
};

std::string candidateToString(PreconditioningCandidate candidate);
PreconditioningCandidate candidateFromString(const std::string& name);

struct StreamStats {
    double entropy{0.0};
    double rleRatio{0.0};
    double deltaVarRatio{1.0};
    double rangeBitsRatio{1.0};
    uint64_t gcdStep{1};
    double bitPlaneEntropyGradient{0.0};
    double spatialVarRatio{1.0};
    double iwtDetailEnergyRatio{1.0};
    size_t sampleCount{0};
};

struct BtrBlocksSelectorParams {
    size_t microChunkSize{65536}; // 64k elements or bytes
    size_t maxChunksToSample{16}; // Max microchunks sampled
};

class BtrBlocksSelector {
   public:
    explicit BtrBlocksSelector(BtrBlocksSelectorParams params = {});

    // Compute stream statistics for a data buffer
    static StreamStats computeStats(
            const uint8_t* data,
            size_t size,
            size_t elementSize = 1);

    // Evaluate cascading decision tree on stream stats
    static PreconditioningCandidate evaluateDecisionTree(const StreamStats& stats);

    // Select candidate for raw buffer
    PreconditioningCandidate selectCandidate(
            const uint8_t* data,
            size_t size,
            size_t elementSize = 1) const;

    // Select candidate for MultiInput vector
    PreconditioningCandidate selectCandidate(
            const std::vector<MultiInput>& inputs) const;

    // Analyze MultiInput vector and return aggregated StreamStats
    StreamStats analyzeInputs(const std::vector<MultiInput>& inputs) const;

   private:
    BtrBlocksSelectorParams params_;
};

} // namespace openzl::training
