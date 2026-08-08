// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include "openzl/codecs/tans4_interleaved/decode_tans4_interleaved_kernel.h"
#include "openzl/codecs/tans4_interleaved/encode_tans4_interleaved_kernel.h"

namespace {

void fuzzTans4(const uint8_t* input, size_t count, const char* testName)
{
    if (count == 0) {
        size_t bound = ZS_tans4EncodeBound(0);
        std::vector<uint8_t> compressed(bound);
        size_t cSize = ZS_tans4Encode(compressed.data(), bound, nullptr, 0);
        if (cSize == 0) abort();

        size_t dSize = ZS_tans4GetDecompressedSize(compressed.data(), cSize);
        if (dSize != 0) abort();

        size_t decSize = ZS_tans4Decode(nullptr, 0, compressed.data(), cSize, 0);
        if (decSize != 0) abort();
        return;
    }

    size_t bound = ZS_tans4EncodeBound(count);
    std::vector<uint8_t> compressed(bound);

    size_t cSize = ZS_tans4Encode(compressed.data(), bound, input, count);
    if (cSize == 0 || cSize > bound) {
        std::cerr << "Encode failed for count=" << count << " test=" << testName << std::endl;
        abort();
    }

    size_t dSize = ZS_tans4GetDecompressedSize(compressed.data(), cSize);
    if (dSize != count) {
        std::cerr << "GetDecompressedSize mismatch: expected " << count << ", got " << dSize << " test=" << testName << std::endl;
        abort();
    }

    std::vector<uint8_t> decompressed(count, 0);
    size_t decSize = ZS_tans4Decode(decompressed.data(), count, compressed.data(), cSize, count);
    if (decSize != count) {
        std::cerr << "Decode size mismatch: expected " << count << ", got " << decSize << " test=" << testName << std::endl;
        abort();
    }

    if (std::memcmp(decompressed.data(), input, count) != 0) {
        std::cerr << "Roundtrip mismatch for count=" << count << " test=" << testName << std::endl;
        for (size_t i = 0; i < count; ++i) {
            if (decompressed[i] != input[i]) {
                std::cerr << "First mismatch at i=" << i << " expected 0x" << std::hex << (int)input[i]
                          << " got 0x" << (int)decompressed[i] << std::dec << std::endl;
                break;
            }
        }
        abort();
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) {
        fuzzTans4(nullptr, 0, "fuzz_empty");
        return 0;
    }

    fuzzTans4(data, size, "fuzz_raw");
    if (size > 1) fuzzTans4(data, size / 2, "fuzz_half");
    if (size > 3) fuzzTans4(data, 3, "fuzz_3");

    return 0;
}

#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION) && !defined(LIBFUZZER_TEST_MAIN)
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    std::cout << "=== Running fuzz_tans4_interleaved standalone test cases ===" << std::endl;

    // 1. Edge case: empty input
    fuzzTans4(nullptr, 0, "empty");

    // 2. Single byte inputs
    for (int b = 0; b < 256; ++b) {
        uint8_t single = (uint8_t)b;
        fuzzTans4(&single, 1, "single_byte");
    }

    // 3. Small inputs
    for (size_t len : {2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 100, 255, 256, 500, 1000, 1024, 4000, 10000}) {
        std::vector<uint8_t> vec(len);

        // All identical (constant)
        std::fill(vec.begin(), vec.end(), (uint8_t)42);
        fuzzTans4(vec.data(), len, "constant");

        // Sequential pattern
        for (size_t i = 0; i < len; ++i) {
            vec[i] = (uint8_t)(i & 0xFF);
        }
        fuzzTans4(vec.data(), len, "sequential");

        // Alternating pattern
        for (size_t i = 0; i < len; ++i) {
            vec[i] = (i % 2 == 0) ? 0xAA : 0x55;
        }
        fuzzTans4(vec.data(), len, "alternating");

        // Skewed frequency distribution
        for (size_t i = 0; i < len; ++i) {
            uint32_t r = (uint32_t)(i * 2654435761U);
            vec[i] = (uint8_t)(r % 7);
        }
        fuzzTans4(vec.data(), len, "skewed");

        // Full 8-bit pseudo-random
        for (size_t i = 0; i < len; ++i) {
            uint32_t r = (uint32_t)((i + 1) * 1103515245 + 12345);
            vec[i] = (uint8_t)((r >> 16) & 0xFF);
        }
        fuzzTans4(vec.data(), len, "random");
    }

    std::cout << "=== All fuzz_tans4_interleaved tests PASSED 100% ===" << std::endl;
    return 0;
}
#endif
