// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <gtest/gtest.h>
#include <vector>

#include "openzl/openzl.hpp"

namespace openzl {
namespace {

template <typename T>
void testPcodecRoundtrip(const std::vector<T>& input, uint64_t multiplier = 10)
{
    Compressor compressor;
    auto pcodecNode = compressor.registerNode(nodes::PcodecInt{});

    // Pipeline: Pcodec -> (latents, residuals) -> Store
    auto inputGraph = compressor.inputGraph(Type::Numeric);
    auto pcodecGraph = compressor.addGraph(pcodecNode, inputGraph);

    auto storeGraph0 = compressor.addGraph(nodes::Store{}, pcodecGraph, 0);
    auto storeGraph1 = compressor.addGraph(nodes::Store{}, pcodecGraph, 1);

    compressor.addGraphOutput(storeGraph0);
    compressor.addGraphOutput(storeGraph1);

    compressor.selectRootGraph(inputGraph);

    CCtx cctx;
    auto inputRef = InputRef(input.data(), input.size());
    auto compressed = cctx.compress(compressor, inputRef);

    DCtx dctx;
    std::vector<T> decompressed(input.size());
    auto outputRef = OutputRef(decompressed.data(), decompressed.size());

    dctx.decompress(outputRef, compressed);

    EXPECT_EQ(input, decompressed);
}

TEST(PcodecTest, RoundtripUint16MultiModal)
{
    std::vector<uint16_t> data;
    // Multi-modal distribution around 1000, 2000, 5000 with small offset noise
    for (size_t i = 0; i < 1000; ++i) {
        uint16_t base = (i % 3 == 0) ? 1000 : (i % 3 == 1) ? 2000 : 5000;
        uint16_t offset = i % 10;
        data.push_back(base + offset);
    }
    testPcodecRoundtrip(data, 10);
}

TEST(PcodecTest, RoundtripUint32)
{
    std::vector<uint32_t> data;
    for (size_t i = 0; i < 2000; ++i) {
        data.push_back(100000u + (i % 50) * 100 + (i % 7));
    }
    testPcodecRoundtrip(data, 100);
}

TEST(PcodecTest, RoundtripUint8)
{
    std::vector<uint8_t> data;
    for (size_t i = 0; i < 500; ++i) {
        data.push_back((i % 5) * 20 + (i % 3));
    }
    testPcodecRoundtrip(data, 20);
}

TEST(PcodecTest, RoundtripUint64)
{
    std::vector<uint64_t> data;
    for (size_t i = 0; i < 1000; ++i) {
        data.push_back(1000000000ULL + (i % 100) * 1000ULL + (i % 9));
    }
    testPcodecRoundtrip(data, 1000);
}

} // namespace
} // namespace openzl
