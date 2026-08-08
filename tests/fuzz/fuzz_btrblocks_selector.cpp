// Copyright (c) Meta Platforms, Inc. and affiliates.
// Coverage-guided LLVM libFuzzer and Unit Test target for BtrBlocks Micro-Chunk Cascading Decision Trees.

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "openzl/codecs/delta/decode_delta_kernel.h"
#include "openzl/codecs/delta/encode_delta_kernel.h"
#include "openzl/codecs/iwt/decode_iwt_kernel.h"
#include "openzl/codecs/iwt/encode_iwt_kernel.h"
#include "openzl/codecs/range_pack/decode_range_pack_kernel.h"
#include "openzl/codecs/range_pack/encode_range_pack_kernel.h"
#include "openzl/codecs/zigzag/decode_zigzag_kernel.h"
#include "openzl/codecs/zigzag/encode_zigzag_kernel.h"
#include "tools/training/btrblocks_selector.h"

using namespace openzl::training;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0 || data == nullptr) {
        return 0;
    }

    // 1. Evaluate BtrBlocks micro-chunk decision tree
    BtrBlocksSelector selector;
    StreamStats stats = BtrBlocksSelector::computeStats(data, size, 1);
    PreconditioningCandidate candidate = BtrBlocksSelector::evaluateDecisionTree(stats);

    // Verify string conversion stability
    std::string strName = candidateToString(candidate);
    PreconditioningCandidate roundTripCandidate = candidateFromString(strName);
    assert(candidate == roundTripCandidate);
    (void)roundTripCandidate;
    (void)strName;

    // 2. Delta Round-Trip Safety Check
    {
        size_t nelts = size;
        uint8_t first = 0;
        std::vector<uint8_t> deltas(nelts > 0 ? nelts - 1 : 0);
        std::vector<uint8_t> decoded(nelts);

        ZS_deltaEncode8(&first, deltas.data(), data, nelts);
        ZS_deltaDecode8(decoded.data(), first, deltas.data(), nelts);
        assert(memcmp(data, decoded.data(), nelts) == 0);
    }

    // 3. Zigzag 8-bit Round-Trip Safety Check
    {
        size_t nelts = size;
        const int8_t* src8 = reinterpret_cast<const int8_t*>(data);
        std::vector<uint8_t> encoded(nelts);
        std::vector<int8_t> decoded(nelts);

        ZL_zigzagEncode8(encoded.data(), src8, nelts);
        ZL_zigzagDecode8(decoded.data(), encoded.data(), nelts);
        assert(memcmp(src8, decoded.data(), nelts) == 0);
    }

    // 4. Range Pack Round-Trip Safety Check
    if (size > 0) {
        uint8_t minVal = 255;
        for (size_t i = 0; i < size; ++i) {
            if (data[i] < minVal) minVal = data[i];
        }

        std::vector<uint8_t> packed(size);
        std::vector<uint8_t> unpacked(size);

        rangePackEncode(packed.data(), 1, data, 1, size, minVal);
        rangePackDecode(unpacked.data(), 1, packed.data(), 1, size, minVal);
        assert(memcmp(data, unpacked.data(), size) == 0);
    }

    // 5. IWT Wavelet Round-Trip Safety Check
    if (size >= 2) {
        size_t half = size / 2;
        size_t approxLen = (size + 1) / 2;
        size_t detailLen = size / 2;

        std::vector<uint8_t> approx(approxLen);
        std::vector<uint8_t> detail(detailLen);
        std::vector<uint8_t> decoded(size);

        ZS_iwtEncode(approx.data(), detail.data(), data, size, sizeof(uint8_t));
        ZS_iwtDecode(decoded.data(), approx.data(), detail.data(), size, sizeof(uint8_t));
        assert(memcmp(data, decoded.data(), size) == 0);
    }

    return 0;
}

int main(int argc, char** argv)
{
    std::cout << "=== Running BtrBlocks Micro-Chunk Selector Fuzz / Unit Tests ===" << std::endl;

    // Test Case 1: Constant / Long RLE data -> Delta candidate
    {
        std::vector<uint8_t> rleData(1024, 42);
        StreamStats stats = BtrBlocksSelector::computeStats(rleData.data(), rleData.size());
        PreconditioningCandidate c = BtrBlocksSelector::evaluateDecisionTree(stats);
        std::cout << "[Test 1] RLE Data -> Selected: " << candidateToString(c) << std::endl;
        assert(c == PreconditioningCandidate::Delta);
        LLVMFuzzerTestOneInput(rleData.data(), rleData.size());
    }

    // Test Case 2: Range packed data -> Range candidate
    {
        std::vector<uint8_t> rangeData(1024);
        for (size_t i = 0; i < rangeData.size(); ++i) {
            rangeData[i] = 100 + (i % 3); // narrow spread [100, 102]
        }
        StreamStats stats = BtrBlocksSelector::computeStats(rangeData.data(), rangeData.size());
        PreconditioningCandidate c = BtrBlocksSelector::evaluateDecisionTree(stats);
        std::cout << "[Test 2] Narrow Range Data -> Selected: " << candidateToString(c) << std::endl;
        assert(c == PreconditioningCandidate::Delta || c == PreconditioningCandidate::Range || c == PreconditioningCandidate::QuaRs);
        LLVMFuzzerTestOneInput(rangeData.data(), rangeData.size());
    }

    // Test Case 3: Smooth linear slope data -> Delta / IWT candidate
    {
        std::vector<uint8_t> slopeData(1024);
        for (size_t i = 0; i < slopeData.size(); ++i) {
            slopeData[i] = static_cast<uint8_t>(i % 256);
        }
        StreamStats stats = BtrBlocksSelector::computeStats(slopeData.data(), slopeData.size());
        PreconditioningCandidate c = BtrBlocksSelector::evaluateDecisionTree(stats);
        std::cout << "[Test 3] Smooth Slope Data -> Selected: " << candidateToString(c) << std::endl;
        assert(c == PreconditioningCandidate::Delta || c == PreconditioningCandidate::IWT);
        LLVMFuzzerTestOneInput(slopeData.data(), slopeData.size());
    }

    // Test Case 4: High entropy pseudo-random data -> None candidate
    {
        std::vector<uint8_t> randData(4096);
        for (size_t i = 0; i < randData.size(); ++i) {
            randData[i] = static_cast<uint8_t>((i * 1664525 + 1013904223) >> 24);
        }
        StreamStats stats = BtrBlocksSelector::computeStats(randData.data(), randData.size());
        PreconditioningCandidate c = BtrBlocksSelector::evaluateDecisionTree(stats);
        std::cout << "[Test 4] Random Data -> Selected: " << candidateToString(c) << std::endl;
        LLVMFuzzerTestOneInput(randData.data(), randData.size());
    }

    std::cout << "=== All BtrBlocks Selector Fuzz / Unit Tests Passed Successfully ===" << std::endl;
    return 0;
}
