// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <gtest/gtest.h>
#include <numeric>
#include <vector>

#include "openzl/openzl.hpp"
#include "tests/codecs/test_codec.h"

namespace openzl {
namespace tests {

class PcodecTest : public CodecTest {
   public:
    template <typename T>
    void testPcodecRoundtrip(const std::vector<T>& input)
    {
        compressor_.setParameter(CParam::FormatVersion, ZL_MAX_FORMAT_VERSION);

        auto store0 = graphs::Store{}(compressor_);
        auto store1 = graphs::Store{}(compressor_);
        auto pcodecGraph = nodes::PcodecInt{}(compressor_, store0, store1);
        compressor_.selectStartingGraph(pcodecGraph);

        Input numericInput = Input::refNumeric(poly::span<const T>{ input });
        testRoundTrip(numericInput);
    }
};

TEST_F(PcodecTest, RoundtripUint16MultiModal)
{
    std::vector<uint16_t> data;
    for (size_t i = 0; i < 1000; ++i) {
        uint16_t base = (i % 3 == 0) ? 1000 : (i % 3 == 1) ? 2000 : 5000;
        uint16_t offset = i % 10;
        data.push_back(base + offset);
    }
    testPcodecRoundtrip(data);
}

TEST_F(PcodecTest, RoundtripUint32)
{
    std::vector<uint32_t> data;
    for (size_t i = 0; i < 2000; ++i) {
        data.push_back(100000u + (i % 50) * 100 + (i % 7));
    }
    testPcodecRoundtrip(data);
}

TEST_F(PcodecTest, RoundtripUint8)
{
    std::vector<uint8_t> data;
    for (size_t i = 0; i < 500; ++i) {
        data.push_back((i % 5) * 20 + (i % 3));
    }
    testPcodecRoundtrip(data);
}

TEST_F(PcodecTest, RoundtripUint64)
{
    std::vector<uint64_t> data;
    for (size_t i = 0; i < 1000; ++i) {
        data.push_back(1000000000ULL + (i % 100) * 1000ULL + (i % 9));
    }
    testPcodecRoundtrip(data);
}

} // namespace tests
} // namespace openzl
