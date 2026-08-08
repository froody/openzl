// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "openzl/codecs/linear_predict/decode_linear_predict_kernel.h"
#include "openzl/codecs/linear_predict/encode_linear_predict_kernel.h"

namespace {

template <typename T>
void fuzzLinearPredictTyped(
        void (*encodeFn)(T*, const T*, size_t, size_t, uint8_t),
        void (*decodeFn)(T*, const T*, size_t, size_t, uint8_t),
        const T* input,
        size_t count,
        size_t stride,
        uint8_t mode)
{
    if (count == 0) {
        encodeFn(nullptr, nullptr, 0, stride, mode);
        decodeFn(nullptr, nullptr, 0, stride, mode);
        ZS_linearPredictEncode(nullptr, nullptr, 0, sizeof(T), stride, mode);
        ZS_linearPredictDecode(nullptr, nullptr, 0, sizeof(T), stride, mode);
        return;
    }

    std::vector<T> encoded(count, static_cast<T>(0));
    std::vector<T> decoded(count, static_cast<T>(0));

    // 1. Direct typed kernel encode/decode roundtrip
    encodeFn(encoded.data(), input, count, stride, mode);
    decodeFn(decoded.data(), encoded.data(), count, stride, mode);

    if (std::memcmp(decoded.data(), input, count * sizeof(T)) != 0) {
        std::abort(); // Fuzzer crash trigger on roundtrip mismatch
    }

    // 2. Generic ZS_linearPredictEncode / ZS_linearPredictDecode roundtrip
    std::fill(encoded.begin(), encoded.end(), static_cast<T>(0));
    std::fill(decoded.begin(), decoded.end(), static_cast<T>(0));

    ZS_linearPredictEncode(encoded.data(), input, count, sizeof(T), stride, mode);
    ZS_linearPredictDecode(decoded.data(), encoded.data(), count, sizeof(T), stride, mode);

    if (std::memcmp(decoded.data(), input, count * sizeof(T)) != 0) {
        std::abort(); // Fuzzer crash trigger on roundtrip mismatch
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 4) {
        // Test zero / minimal input edge cases
        fuzzLinearPredictTyped<uint8_t>(ZS_linearPredictEncode8, ZS_linearPredictDecode8, nullptr, 0, 1, 0);
        fuzzLinearPredictTyped<uint16_t>(ZS_linearPredictEncode16, ZS_linearPredictDecode16, nullptr, 0, 1, 0);
        fuzzLinearPredictTyped<uint32_t>(ZS_linearPredictEncode32, ZS_linearPredictDecode32, nullptr, 0, 1, 0);
        fuzzLinearPredictTyped<uint64_t>(ZS_linearPredictEncode64, ZS_linearPredictDecode64, nullptr, 0, 1, 0);
        return 0;
    }

    // Use header bytes to configure fuzzer run parameters
    uint8_t const widthIdx = data[0] % 4;
    uint8_t const mode = data[1] % 4;
    size_t const stride = (data[2] == 0) ? 1 : static_cast<size_t>(data[2]);

    const uint8_t* payload = data + 3;
    size_t const payloadSize = size - 3;

    switch (widthIdx) {
        case 0: { // uint8_t
            size_t const count = payloadSize / sizeof(uint8_t);
            std::vector<uint8_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint8_t));
            }
            fuzzLinearPredictTyped<uint8_t>(
                    ZS_linearPredictEncode8,
                    ZS_linearPredictDecode8,
                    vec.data(),
                    count,
                    stride,
                    mode);
            break;
        }
        case 1: { // uint16_t
            size_t const count = payloadSize / sizeof(uint16_t);
            std::vector<uint16_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint16_t));
            }
            fuzzLinearPredictTyped<uint16_t>(
                    ZS_linearPredictEncode16,
                    ZS_linearPredictDecode16,
                    vec.data(),
                    count,
                    stride,
                    mode);
            break;
        }
        case 2: { // uint32_t
            size_t const count = payloadSize / sizeof(uint32_t);
            std::vector<uint32_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint32_t));
            }
            fuzzLinearPredictTyped<uint32_t>(
                    ZS_linearPredictEncode32,
                    ZS_linearPredictDecode32,
                    vec.data(),
                    count,
                    stride,
                    mode);
            break;
        }
        case 3: { // uint64_t
            size_t const count = payloadSize / sizeof(uint64_t);
            std::vector<uint64_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint64_t));
            }
            fuzzLinearPredictTyped<uint64_t>(
                    ZS_linearPredictEncode64,
                    ZS_linearPredictDecode64,
                    vec.data(),
                    count,
                    stride,
                    mode);
            break;
        }
    }

    return 0;
}

#if defined(STANDALONE_FUZZER)
int main(int argc, char** argv)
{
    // Standalone main for CMake test execution when not linked with libFuzzer
    (void)argc;
    (void)argv;
    uint8_t testData[256];
    for (size_t i = 0; i < sizeof(testData); ++i) {
        testData[i] = static_cast<uint8_t>((i * 37 + 13) & 0xFF);
    }

    // Run fuzzer test function on synthetic pattern
    LLVMFuzzerTestOneInput(testData, sizeof(testData));

    // Also run on various small sizes and parameters
    for (size_t len = 0; len < 32; ++len) {
        LLVMFuzzerTestOneInput(testData, len);
    }

    return 0;
}
#endif
