// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "tools/training/btrblocks_selector.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <cinttypes>

namespace openzl::training {

std::string candidateToString(PreconditioningCandidate candidate)
{
    switch (candidate) {
        case PreconditioningCandidate::None:
            return "None";
        case PreconditioningCandidate::Morton:
            return "Morton";
        case PreconditioningCandidate::Range:
            return "Range";
        case PreconditioningCandidate::QuaRs:
            return "QuaRs";
        case PreconditioningCandidate::BitPlane:
            return "BitPlane";
        case PreconditioningCandidate::IWT:
            return "IWT";
        case PreconditioningCandidate::Delta:
            return "Delta";
    }
    return "None";
}

PreconditioningCandidate candidateFromString(const std::string& name)
{
    if (name == "Morton") return PreconditioningCandidate::Morton;
    if (name == "Range") return PreconditioningCandidate::Range;
    if (name == "QuaRs") return PreconditioningCandidate::QuaRs;
    if (name == "BitPlane") return PreconditioningCandidate::BitPlane;
    if (name == "IWT") return PreconditioningCandidate::IWT;
    if (name == "Delta") return PreconditioningCandidate::Delta;
    return PreconditioningCandidate::None;
}

BtrBlocksSelector::BtrBlocksSelector(BtrBlocksSelectorParams params)
    : params_(params)
{
}

static uint64_t gcdImpl(uint64_t a, uint64_t b)
{
    while (b != 0) {
        uint64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

StreamStats BtrBlocksSelector::computeStats(
        const uint8_t* data,
        size_t size,
        size_t elementSize)
{
    StreamStats stats;
    if (!data || size == 0) {
        return stats;
    }

    if (elementSize == 0) {
        elementSize = 1;
    }

    size_t numElements = size / elementSize;
    if (numElements == 0) {
        return stats;
    }

    stats.sampleCount = numElements;

    // 1. Entropy & Byte Distribution
    uint64_t byteCounts[256] = {0};
    for (size_t i = 0; i < size; ++i) {
        byteCounts[data[i]]++;
    }

    double entropy = 0.0;
    double totalBytes = static_cast<double>(size);
    for (int i = 0; i < 256; ++i) {
        if (byteCounts[i] > 0) {
            double p = static_cast<double>(byteCounts[i]) / totalBytes;
            entropy -= p * (std::log2(p));
        }
    }
    stats.entropy = entropy;

    // 2. RLE ratio & element-wise differences
    size_t rleMatches = 0;
    uint8_t minByte = 255;
    uint8_t maxByte = 0;

    for (size_t i = 0; i < size; ++i) {
        if (data[i] < minByte) minByte = data[i];
        if (data[i] > maxByte) maxByte = data[i];
    }

    if (numElements > 1) {
        if (elementSize == 1) {
            for (size_t i = 1; i < size; ++i) {
                if (data[i] == data[i - 1]) {
                    rleMatches++;
                }
            }
            stats.rleRatio = static_cast<double>(rleMatches) / static_cast<double>(size - 1);
        } else {
            for (size_t i = 1; i < numElements; ++i) {
                if (std::equal(data + (i - 1) * elementSize, data + i * elementSize, data + i * elementSize)) {
                    rleMatches++;
                }
            }
            stats.rleRatio = static_cast<double>(rleMatches) / static_cast<double>(numElements - 1);
        }
    }

    // 3. Raw Mean, Variance, Delta Variance
    double sumX = 0.0;
    double sumX2 = 0.0;
    for (size_t i = 0; i < size; ++i) {
        double v = static_cast<double>(data[i]);
        sumX += v;
        sumX2 += v * v;
    }
    double meanX = sumX / totalBytes;
    double varX = (sumX2 / totalBytes) - (meanX * meanX);
    if (varX < 0.0) varX = 0.0;

    if (size > 1) {
        double sumD = 0.0;
        double sumD2 = 0.0;
        size_t nD = size - 1;
        for (size_t i = 1; i < size; ++i) {
            double d = static_cast<double>(data[i]) - static_cast<double>(data[i - 1]);
            sumD += d;
            sumD2 += d * d;
        }
        double meanD = sumD / static_cast<double>(nD);
        double varD = (sumD2 / static_cast<double>(nD)) - (meanD * meanD);
        if (varD < 0.0) varD = 0.0;

        stats.deltaVarRatio = (varX > 1e-6) ? (varD / varX) : 1.0;
    }

    // 4. Range bits ratio
    uint32_t spread = maxByte - minByte;
    uint32_t spreadBits = 0;
    while (spread > 0) {
        spreadBits++;
        spread >>= 1;
    }
    stats.rangeBitsRatio = static_cast<double>(spreadBits) / 8.0;

    // 5. GCD step calculation
    uint64_t currentGcd = 0;
    for (size_t i = 1; i < size; ++i) {
        int diff = std::abs(static_cast<int>(data[i]) - static_cast<int>(data[i - 1]));
        if (diff > 0) {
            currentGcd = gcdImpl(currentGcd, static_cast<uint64_t>(diff));
            if (currentGcd == 1) break;
        }
    }
    stats.gcdStep = (currentGcd > 0) ? currentGcd : 1;

    // 6. Bit-plane entropy gradient
    uint64_t highNibble[16] = {0};
    uint64_t lowNibble[16] = {0};
    for (size_t i = 0; i < size; ++i) {
        highNibble[(data[i] >> 4) & 0x0F]++;
        lowNibble[data[i] & 0x0F]++;
    }
    double hUpper = 0.0;
    double hLower = 0.0;
    for (int i = 0; i < 16; ++i) {
        if (highNibble[i] > 0) {
            double p = static_cast<double>(highNibble[i]) / totalBytes;
            hUpper -= p * std::log2(p);
        }
        if (lowNibble[i] > 0) {
            double p = static_cast<double>(lowNibble[i]) / totalBytes;
            hLower -= p * std::log2(p);
        }
    }
    stats.bitPlaneEntropyGradient = std::max(0.0, (hLower - hUpper) / 4.0);

    // 7. Spatial variance ratio (2D grid stride)
    size_t stride = static_cast<size_t>(std::sqrt(static_cast<double>(size)));
    if (stride < 8) stride = 8;
    if (stride > 256) stride = 256;

    if (size > stride) {
        double sumS2 = 0.0;
        size_t nS = size - stride;
        for (size_t i = 0; i < nS; ++i) {
            double diff = static_cast<double>(data[i + stride]) - static_cast<double>(data[i]);
            sumS2 += diff * diff;
        }
        double varS = sumS2 / static_cast<double>(nS);
        stats.spatialVarRatio = (varX > 1e-6) ? (varS / varX) : 1.0;
    }

    // 8. IWT Detail Energy Ratio
    if (size >= 2) {
        double detailEnergy = 0.0;
        double signalEnergy = 0.0;
        size_t pairs = size / 2;
        for (size_t i = 0; i < pairs; ++i) {
            double x0 = static_cast<double>(data[2 * i]);
            double x1 = static_cast<double>(data[2 * i + 1]);
            double detail = x1 - x0;
            detailEnergy += detail * detail;
            signalEnergy += x0 * x0 + x1 * x1;
        }
        signalEnergy /= static_cast<double>(size);
        detailEnergy /= static_cast<double>(pairs);
        stats.iwtDetailEnergyRatio = (signalEnergy > 1e-6) ? (detailEnergy / signalEnergy) : 1.0;
    }

    return stats;
}

PreconditioningCandidate BtrBlocksSelector::evaluateDecisionTree(const StreamStats& stats)
{
    if (stats.sampleCount == 0) {
        return PreconditioningCandidate::None;
    }

    if (stats.entropy > 7.75 && stats.rleRatio < 0.05) {
        return PreconditioningCandidate::None;
    }

    if (stats.rleRatio > 0.35 || stats.deltaVarRatio < 0.25) {
        return PreconditioningCandidate::Delta;
    }

    if (stats.gcdStep > 1 && stats.rangeBitsRatio < 0.85) {
        return PreconditioningCandidate::QuaRs;
    }

    if (stats.rangeBitsRatio < 0.55) {
        return PreconditioningCandidate::Range;
    }

    if (stats.bitPlaneEntropyGradient > 0.45) {
        return PreconditioningCandidate::BitPlane;
    }

    if (stats.spatialVarRatio < 0.35) {
        return PreconditioningCandidate::Morton;
    }

    if (stats.iwtDetailEnergyRatio < 0.20) {
        return PreconditioningCandidate::IWT;
    }

    if (stats.rangeBitsRatio < 0.80) {
        return PreconditioningCandidate::Range;
    }

    return PreconditioningCandidate::None;
}

PreconditioningCandidate BtrBlocksSelector::selectCandidate(
        const uint8_t* data,
        size_t size,
        size_t elementSize) const
{
    StreamStats stats = computeStats(data, size, elementSize);
    return evaluateDecisionTree(stats);
}

PreconditioningCandidate BtrBlocksSelector::selectCandidate(
        const std::vector<MultiInput>& inputs) const
{
    StreamStats stats = analyzeInputs(inputs);
    return evaluateDecisionTree(stats);
}

StreamStats BtrBlocksSelector::analyzeInputs(const std::vector<MultiInput>& inputs) const
{
    StreamStats aggregated;
    size_t chunkCount = 0;

    for (const auto& multiInput : inputs) {
        for (const auto& input : *multiInput) {
            const uint8_t* ptr = static_cast<const uint8_t*>(input.ptr());
            size_t size = input.contentSize();
            if (!ptr || size == 0) continue;

            size_t chunkSize = params_.microChunkSize;
            size_t numChunks = (size + chunkSize - 1) / chunkSize;
            size_t step = std::max<size_t>(1, numChunks / params_.maxChunksToSample);

            for (size_t i = 0; i < size; i += step * chunkSize) {
                size_t currentChunkSize = std::min(chunkSize, size - i);
                StreamStats chunkStats = computeStats(
                        ptr + i,
                        currentChunkSize,
                        input.eltWidth() ? input.eltWidth() : 1);

                aggregated.entropy += chunkStats.entropy;
                aggregated.rleRatio += chunkStats.rleRatio;
                aggregated.deltaVarRatio += chunkStats.deltaVarRatio;
                aggregated.rangeBitsRatio += chunkStats.rangeBitsRatio;
                aggregated.gcdStep = gcdImpl(aggregated.gcdStep, chunkStats.gcdStep);
                aggregated.bitPlaneEntropyGradient += chunkStats.bitPlaneEntropyGradient;
                aggregated.spatialVarRatio += chunkStats.spatialVarRatio;
                aggregated.iwtDetailEnergyRatio += chunkStats.iwtDetailEnergyRatio;
                aggregated.sampleCount += chunkStats.sampleCount;

                chunkCount++;
                if (chunkCount >= params_.maxChunksToSample * inputs.size()) break;
            }
        }
    }

    if (chunkCount > 0) {
        aggregated.entropy /= chunkCount;
        aggregated.rleRatio /= chunkCount;
        aggregated.deltaVarRatio /= chunkCount;
        aggregated.rangeBitsRatio /= chunkCount;
        aggregated.bitPlaneEntropyGradient /= chunkCount;
        aggregated.spatialVarRatio /= chunkCount;
        aggregated.iwtDetailEnergyRatio /= chunkCount;
    }

    return aggregated;
}

} // namespace openzl::training
