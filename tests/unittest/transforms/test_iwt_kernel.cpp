// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <vector>
#include <numeric>
#include <gtest/gtest.h>

#include "openzl/codecs/iwt/decode_iwt_kernel.h"
#include "openzl/codecs/iwt/encode_iwt_kernel.h"

namespace {
template <typename T>
class IwtKernelTest : public testing::Test {
   public:
    void testInput(std::vector<T> const& input)
    {
        if (input.empty()) return;
        size_t const nbApprox = (input.size() + 1) / 2;
        size_t const nbDetail = input.size() / 2;

        std::vector<T> approx(nbApprox);
        std::vector<T> detail(nbDetail);
        std::vector<T> output(input.size());

        ZS_iwtEncode(approx.data(), detail.data(), input.data(), input.size(), sizeof(T));
        ZS_iwtDecode(output.data(), approx.data(), detail.data(), input.size(), sizeof(T));

        EXPECT_EQ(output, input);
    }

    T base = T(1) << (sizeof(T) * 8 - 1);
};

using IwtTypes = ::testing::Types<uint8_t, uint16_t, uint32_t, uint64_t>;
TYPED_TEST_SUITE(IwtKernelTest, IwtTypes, );

TYPED_TEST(IwtKernelTest, EmptyRoundTrip)
{
    ZS_iwtEncode(nullptr, nullptr, nullptr, 0, sizeof(TypeParam));
    ZS_iwtDecode(nullptr, nullptr, nullptr, 0, sizeof(TypeParam));
}

TYPED_TEST(IwtKernelTest, SingleElementRoundTrip)
{
    std::vector<TypeParam> input = { this->base };
    this->testInput(input);
}

TYPED_TEST(IwtKernelTest, TwoElementsRoundTrip)
{
    std::vector<TypeParam> input = { TypeParam(100), TypeParam(150) };
    this->testInput(input);
}

TYPED_TEST(IwtKernelTest, OddLengthRoundTrip)
{
    std::vector<TypeParam> input = { TypeParam(10), TypeParam(20), TypeParam(30), TypeParam(40), TypeParam(50) };
    this->testInput(input);
}

TYPED_TEST(IwtKernelTest, LinearSlopeZeroDetailProperty)
{
    using T = TypeParam;
    std::vector<T> input(10);
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = (T)(10 * i + 5);
    }

    size_t const nbApprox = (input.size() + 1) / 2;
    size_t const nbDetail = input.size() / 2;

    std::vector<T> approx(nbApprox);
    std::vector<T> detail(nbDetail);
    std::vector<T> output(input.size());

    ZS_iwtEncode(approx.data(), detail.data(), input.data(), input.size(), sizeof(T));

    for (size_t d = 0; d + 1 < detail.size(); ++d) {
        EXPECT_EQ(detail[d], (T)0);
    }

    ZS_iwtDecode(output.data(), approx.data(), detail.data(), input.size(), sizeof(T));
    EXPECT_EQ(output, input);
}

TYPED_TEST(IwtKernelTest, ArbitraryRoundTrip)
{
    std::vector<TypeParam> input = {
        TypeParam(this->base),
        TypeParam(this->base + 10),
        TypeParam(this->base + 25),
        TypeParam(this->base + 38),
        TypeParam(this->base + 51),
        TypeParam(this->base + 60),
        0,
        100,
        200,
        255,
    };
    this->testInput(input);
}
} // namespace
