// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

#include "openzl/codecs/elf_erase/decode_elf_erase_kernel.h"
#include "openzl/codecs/elf_erase/encode_elf_erase_kernel.h"

namespace {

template <typename T>
void fuzzElfEraseTyped(const T* input, size_t count)
{
    size_t const eltWidth = sizeof(T);
    if (count == 0) {
        ZS_elfEraseEncode(nullptr, 0, nullptr, 0, eltWidth);
        ZS_elfEraseDecode(nullptr, nullptr, 0, 0, eltWidth);
        return;
    }

    size_t const maxCapacity = count * (eltWidth + 4) + 64;
    std::vector<uint8_t> encoded(maxCapacity);
    std::vector<T> decoded(count, static_cast<T>(0));

    // 1. Direct typed kernel encode/decode roundtrip
    size_t bytesWritten = 0;
    if (sizeof(T) == 4) {
        const void* srcVoid = static_cast<const void*>(input);
        void* dstVoid = static_cast<void*>(decoded.data());
        bytesWritten = ZS_elfEraseEncode32(encoded.data(), maxCapacity, static_cast<const uint32_t*>(srcVoid), count);
        assert(bytesWritten > 0);

        int const ok = ZS_elfEraseDecode32(static_cast<uint32_t*>(dstVoid), encoded.data(), bytesWritten, count);
        (void)ok;
        assert(ok == 1);
    } else if (sizeof(T) == 8) {
        const void* srcVoid = static_cast<const void*>(input);
        void* dstVoid = static_cast<void*>(decoded.data());
        bytesWritten = ZS_elfEraseEncode64(encoded.data(), maxCapacity, static_cast<const uint64_t*>(srcVoid), count);
        assert(bytesWritten > 0);

        int const ok = ZS_elfEraseDecode64(static_cast<uint64_t*>(dstVoid), encoded.data(), bytesWritten, count);
        (void)ok;
        assert(ok == 1);
    }

    if (memcmp(decoded.data(), input, count * eltWidth) != 0) {
        std::fprintf(stderr, "fuzz_elf_erase roundtrip mismatch for element width %zu, count %zu\n", eltWidth, count);
        std::abort();
    }

    // 2. Generic ZS_elfEraseEncode / ZS_elfEraseDecode roundtrip
    std::fill(encoded.begin(), encoded.end(), 0);
    std::fill(decoded.begin(), decoded.end(), static_cast<T>(0));

    bytesWritten = ZS_elfEraseEncode(encoded.data(), maxCapacity, input, count, eltWidth);
    assert(bytesWritten > 0);

    int const ok = ZS_elfEraseDecode(decoded.data(), encoded.data(), bytesWritten, count, eltWidth);
    (void)ok;
    assert(ok == 1);

    if (memcmp(decoded.data(), input, count * eltWidth) != 0) {
        std::fprintf(stderr, "fuzz_elf_erase generic roundtrip mismatch for element width %zu, count %zu\n", eltWidth, count);
        std::abort();
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) {
        fuzzElfEraseTyped<uint32_t>(nullptr, 0);
        fuzzElfEraseTyped<uint64_t>(nullptr, 0);
        return 0;
    }

    uint8_t const mode = data[0] % 2;
    const uint8_t* payload = data + 1;
    size_t const payloadSize = size - 1;

    if (mode == 0) {
        size_t const count = payloadSize / sizeof(uint32_t);
        std::vector<uint32_t> vec(count);
        if (count > 0) {
            std::memcpy(vec.data(), payload, count * sizeof(uint32_t));
        }
        fuzzElfEraseTyped<uint32_t>(vec.data(), count);
    } else {
        size_t const count = payloadSize / sizeof(uint64_t);
        std::vector<uint64_t> vec(count);
        if (count > 0) {
            std::memcpy(vec.data(), payload, count * sizeof(uint64_t));
        }
        fuzzElfEraseTyped<uint64_t>(vec.data(), count);
    }

    return 0;
}

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    std::printf("Running fuzz_elf_erase self-test suite...\n");

    // Edge case: empty input
    LLVMFuzzerTestOneInput(nullptr, 0);

    // Test cases with synthetic float / double geometry streams
    std::mt19937 rng(42);

    for (int run = 0; run < 1000; ++run) {
        size_t const num32 = (run % 50) * 17 + 1;
        std::vector<float> floats(num32);
        float base = 123.456f;
        for (size_t i = 0; i < num32; ++i) {
            if (i % 5 == 0) {
                floats[i] = base;
            } else {
                base += (rng() % 100) * 0.001f;
                floats[i] = base;
            }
        }
        fuzzElfEraseTyped<float>(floats.data(), num32);

        size_t const num64 = (run % 50) * 13 + 1;
        std::vector<double> doubles(num64);
        double dbase = 9876.543210123;
        for (size_t i = 0; i < num64; ++i) {
            if (i % 4 == 0) {
                doubles[i] = dbase;
            } else {
                dbase += (rng() % 1000) * 0.00001;
                doubles[i] = dbase;
            }
        }
        fuzzElfEraseTyped<double>(doubles.data(), num64);

        // Random raw byte payloads fed to LLVMFuzzerTestOneInput
        size_t const bytesLen = (rng() % 512) + 2;
        std::vector<uint8_t> randBytes(bytesLen);
        for (size_t i = 0; i < bytesLen; ++i) {
            randBytes[i] = static_cast<uint8_t>(rng() & 0xFF);
        }
        LLVMFuzzerTestOneInput(randBytes.data(), randBytes.size());
    }

    std::printf("fuzz_elf_erase: 100%% round-trip verification SUCCESS!\n");
    return 0;
}
#endif
