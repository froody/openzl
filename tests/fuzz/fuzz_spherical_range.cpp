// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "openzl/codecs/spherical_range/decode_spherical_range_kernel.h"
#include "openzl/codecs/spherical_range/encode_spherical_range_kernel.h"

namespace {

template <typename T>
void fuzzSphericalRangeTyped(
        void (*encodeFn)(T*, const T*, size_t),
        void (*decodeFn)(T*, const T*, size_t),
        const T* input,
        size_t count)
{
    if (count == 0) {
        encodeFn(nullptr, nullptr, 0);
        decodeFn(nullptr, nullptr, 0);
        ZS_sphericalRangeEncode(nullptr, nullptr, 0, sizeof(T));
        ZS_sphericalRangeDecode(nullptr, nullptr, 0, sizeof(T));
        return;
    }

    std::vector<T> encoded(count);
    std::vector<T> output(count, static_cast<T>(0));

    // 1. Direct typed kernel encode/decode roundtrip
    encodeFn(encoded.data(), input, count);
    decodeFn(output.data(), encoded.data(), count);

    if (memcmp(output.data(), input, count * sizeof(T)) != 0) {
        abort(); // Fuzzer crash trigger on roundtrip mismatch
    }

    // 2. Generic ZS_sphericalRangeEncode / ZS_sphericalRangeDecode roundtrip
    std::fill(encoded.begin(), encoded.end(), static_cast<T>(0));
    std::fill(output.begin(), output.end(), static_cast<T>(0));

    ZS_sphericalRangeEncode(encoded.data(), input, count, sizeof(T));
    ZS_sphericalRangeDecode(output.data(), encoded.data(), count, sizeof(T));

    if (memcmp(output.data(), input, count * sizeof(T)) != 0) {
        abort(); // Fuzzer crash trigger on roundtrip mismatch
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) {
        fuzzSphericalRangeTyped<uint8_t>(
                ZS_sphericalRangeEncode8, ZS_sphericalRangeDecode8, nullptr, 0);
        fuzzSphericalRangeTyped<uint16_t>(
                ZS_sphericalRangeEncode16, ZS_sphericalRangeDecode16, nullptr, 0);
        fuzzSphericalRangeTyped<uint32_t>(
                ZS_sphericalRangeEncode32, ZS_sphericalRangeDecode32, nullptr, 0);
        fuzzSphericalRangeTyped<uint64_t>(
                ZS_sphericalRangeEncode64, ZS_sphericalRangeDecode64, nullptr, 0);
        return 0;
    }

    uint8_t const mode = data[0] % 4;
    const uint8_t* payload = data + 1;
    size_t const payloadSize = size - 1;

    switch (mode) {
        case 0: { // uint8_t (1 byte)
            size_t const count = payloadSize / sizeof(uint8_t);
            std::vector<uint8_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint8_t));
            }
            fuzzSphericalRangeTyped<uint8_t>(
                    ZS_sphericalRangeEncode8,
                    ZS_sphericalRangeDecode8,
                    vec.data(),
                    count);
            break;
        }
        case 1: { // uint16_t (2 bytes)
            size_t const count = payloadSize / sizeof(uint16_t);
            std::vector<uint16_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint16_t));
            }
            fuzzSphericalRangeTyped<uint16_t>(
                    ZS_sphericalRangeEncode16,
                    ZS_sphericalRangeDecode16,
                    vec.data(),
                    count);
            break;
        }
        case 2: { // uint32_t (4 bytes)
            size_t const count = payloadSize / sizeof(uint32_t);
            std::vector<uint32_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint32_t));
            }
            fuzzSphericalRangeTyped<uint32_t>(
                    ZS_sphericalRangeEncode32,
                    ZS_sphericalRangeDecode32,
                    vec.data(),
                    count);
            break;
        }
        case 3: { // uint64_t (8 bytes)
            size_t const count = payloadSize / sizeof(uint64_t);
            std::vector<uint64_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint64_t));
            }
            fuzzSphericalRangeTyped<uint64_t>(
                    ZS_sphericalRangeEncode64,
                    ZS_sphericalRangeDecode64,
                    vec.data(),
                    count);
            break;
        }
    }

    return 0;
}

#if !defined(__LIBFUZZER_COMPILER__) && !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
int main()
{
    // Standalone driver when not compiling under libFuzzer
    uint8_t testInput[] = {
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120,
        1, 10, 0, 20, 0, 30, 0, 40, 0, 50, 0, 60, 0, 70, 0,
        2, 100, 0, 0, 0, 200, 0, 0, 0, 44, 1, 0, 0,
        3, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0
    };
    LLVMFuzzerTestOneInput(testInput, sizeof(testInput));

    // Run randomized test vectors
    srand(42);
    for (int iter = 0; iter < 1000; ++iter) {
        size_t sz = rand() % 1000 + 1;
        std::vector<uint8_t> buf(sz);
        for (size_t i = 0; i < sz; ++i) {
            buf[i] = rand() % 256;
        }
        LLVMFuzzerTestOneInput(buf.data(), buf.size());
    }
    return 0;
}
#endif
