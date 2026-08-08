// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "openzl/codecs/morton_3d/decode_morton_3d_kernel.h"
#include "openzl/codecs/morton_3d/encode_morton_3d_kernel.h"

namespace {

template <typename T>
void fuzzMorton3dTyped(
        void (*encodeFn)(uint64_t*, const T*, const T*, const T*, size_t),
        void (*decodeFn)(T*, T*, T*, const uint64_t*, size_t),
        void (*encodeInterleavedFn)(uint64_t*, const T*, size_t),
        void (*decodeInterleavedFn)(T*, const uint64_t*, size_t),
        const T* inputX,
        const T* inputY,
        const T* inputZ,
        size_t count)
{
    if (count == 0) {
        encodeFn(nullptr, nullptr, nullptr, nullptr, 0);
        decodeFn(nullptr, nullptr, nullptr, nullptr, 0);
        encodeInterleavedFn(nullptr, nullptr, 0);
        decodeInterleavedFn(nullptr, nullptr, 0);
        ZS_morton3dEncode(nullptr, nullptr, nullptr, nullptr, 0, sizeof(T));
        ZS_morton3dDecode(nullptr, nullptr, nullptr, nullptr, 0, sizeof(T));
        ZS_morton3dEncodeInterleaved(nullptr, nullptr, 0, sizeof(T));
        ZS_morton3dDecodeInterleaved(nullptr, nullptr, 0, sizeof(T));
        return;
    }

    std::vector<uint64_t> codes(count, 0);
    std::vector<T> outputX(count, static_cast<T>(0));
    std::vector<T> outputY(count, static_cast<T>(0));
    std::vector<T> outputZ(count, static_cast<T>(0));

    // Mask expected output for 32-bit and 64-bit to 21 bits (since 64-bit Morton code holds 21 bits per coordinate)
    std::vector<T> expectedX(count);
    std::vector<T> expectedY(count);
    std::vector<T> expectedZ(count);
    for (size_t i = 0; i < count; ++i) {
        if (sizeof(T) <= 2) {
            expectedX[i] = inputX[i];
            expectedY[i] = inputY[i];
            expectedZ[i] = inputZ[i];
        } else {
            expectedX[i] = static_cast<T>(static_cast<uint64_t>(inputX[i]) & 0x1fffffULL);
            expectedY[i] = static_cast<T>(static_cast<uint64_t>(inputY[i]) & 0x1fffffULL);
            expectedZ[i] = static_cast<T>(static_cast<uint64_t>(inputZ[i]) & 0x1fffffULL);
        }
    }

    // 1. Direct typed kernel encode/decode round-trip (separate streams)
    encodeFn(codes.data(), inputX, inputY, inputZ, count);
    decodeFn(outputX.data(), outputY.data(), outputZ.data(), codes.data(), count);

    if (memcmp(outputX.data(), expectedX.data(), count * sizeof(T)) != 0 ||
        memcmp(outputY.data(), expectedY.data(), count * sizeof(T)) != 0 ||
        memcmp(outputZ.data(), expectedZ.data(), count * sizeof(T)) != 0) {
        abort(); // Fuzzer crash trigger on roundtrip mismatch
    }

    // 2. Generic ZS_morton3dEncode / ZS_morton3dDecode round-trip
    std::fill(codes.begin(), codes.end(), 0ULL);
    std::fill(outputX.begin(), outputX.end(), static_cast<T>(0));
    std::fill(outputY.begin(), outputY.end(), static_cast<T>(0));
    std::fill(outputZ.begin(), outputZ.end(), static_cast<T>(0));

    ZS_morton3dEncode(codes.data(), inputX, inputY, inputZ, count, sizeof(T));
    ZS_morton3dDecode(outputX.data(), outputY.data(), outputZ.data(), codes.data(), count, sizeof(T));

    if (memcmp(outputX.data(), expectedX.data(), count * sizeof(T)) != 0 ||
        memcmp(outputY.data(), expectedY.data(), count * sizeof(T)) != 0 ||
        memcmp(outputZ.data(), expectedZ.data(), count * sizeof(T)) != 0) {
        abort(); // Fuzzer crash trigger on roundtrip mismatch
    }

    // 3. Interleaved format round-trip
    std::vector<T> interleavedInput(3 * count);
    std::vector<T> interleavedExpected(3 * count);
    for (size_t i = 0; i < count; ++i) {
        interleavedInput[3 * i]     = inputX[i];
        interleavedInput[3 * i + 1] = inputY[i];
        interleavedInput[3 * i + 2] = inputZ[i];

        interleavedExpected[3 * i]     = expectedX[i];
        interleavedExpected[3 * i + 1] = expectedY[i];
        interleavedExpected[3 * i + 2] = expectedZ[i];
    }

    std::vector<T> interleavedOutput(3 * count, static_cast<T>(0));
    std::fill(codes.begin(), codes.end(), 0ULL);

    encodeInterleavedFn(codes.data(), interleavedInput.data(), count);
    decodeInterleavedFn(interleavedOutput.data(), codes.data(), count);

    if (memcmp(interleavedOutput.data(), interleavedExpected.data(), 3 * count * sizeof(T)) != 0) {
        abort(); // Fuzzer crash trigger on roundtrip mismatch
    }

    // 4. Generic interleaved API
    std::fill(interleavedOutput.begin(), interleavedOutput.end(), static_cast<T>(0));
    std::fill(codes.begin(), codes.end(), 0ULL);

    ZS_morton3dEncodeInterleaved(codes.data(), interleavedInput.data(), count, sizeof(T));
    ZS_morton3dDecodeInterleaved(interleavedOutput.data(), codes.data(), count, sizeof(T));

    if (memcmp(interleavedOutput.data(), interleavedExpected.data(), 3 * count * sizeof(T)) != 0) {
        abort(); // Fuzzer crash trigger on roundtrip mismatch
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) {
        fuzzMorton3dTyped<uint8_t>(
                ZS_morton3dEncode8, ZS_morton3dDecode8,
                ZS_morton3dEncodeInterleaved8, ZS_morton3dDecodeInterleaved8,
                nullptr, nullptr, nullptr, 0);
        fuzzMorton3dTyped<uint16_t>(
                ZS_morton3dEncode16, ZS_morton3dDecode16,
                ZS_morton3dEncodeInterleaved16, ZS_morton3dDecodeInterleaved16,
                nullptr, nullptr, nullptr, 0);
        fuzzMorton3dTyped<uint32_t>(
                ZS_morton3dEncode32, ZS_morton3dDecode32,
                ZS_morton3dEncodeInterleaved32, ZS_morton3dDecodeInterleaved32,
                nullptr, nullptr, nullptr, 0);
        fuzzMorton3dTyped<uint64_t>(
                ZS_morton3dEncode64, ZS_morton3dDecode64,
                ZS_morton3dEncodeInterleaved64, ZS_morton3dDecodeInterleaved64,
                nullptr, nullptr, nullptr, 0);
        return 0;
    }

    uint8_t const mode = data[0] % 4;
    const uint8_t* payload = data + 1;
    size_t const payloadSize = size - 1;

    switch (mode) {
        case 0: { // uint8_t
            size_t const count = payloadSize / (3 * sizeof(uint8_t));
            std::vector<uint8_t> x(count), y(count), z(count);
            if (count > 0) {
                std::memcpy(x.data(), payload, count);
                std::memcpy(y.data(), payload + count, count);
                std::memcpy(z.data(), payload + 2 * count, count);
            }
            fuzzMorton3dTyped<uint8_t>(
                    ZS_morton3dEncode8, ZS_morton3dDecode8,
                    ZS_morton3dEncodeInterleaved8, ZS_morton3dDecodeInterleaved8,
                    x.data(), y.data(), z.data(), count);
            break;
        }
        case 1: { // uint16_t
            size_t const count = payloadSize / (3 * sizeof(uint16_t));
            std::vector<uint16_t> x(count), y(count), z(count);
            if (count > 0) {
                std::memcpy(x.data(), payload, count * sizeof(uint16_t));
                std::memcpy(y.data(), payload + count * sizeof(uint16_t), count * sizeof(uint16_t));
                std::memcpy(z.data(), payload + 2 * count * sizeof(uint16_t), count * sizeof(uint16_t));
            }
            fuzzMorton3dTyped<uint16_t>(
                    ZS_morton3dEncode16, ZS_morton3dDecode16,
                    ZS_morton3dEncodeInterleaved16, ZS_morton3dDecodeInterleaved16,
                    x.data(), y.data(), z.data(), count);
            break;
        }
        case 2: { // uint32_t
            size_t const count = payloadSize / (3 * sizeof(uint32_t));
            std::vector<uint32_t> x(count), y(count), z(count);
            if (count > 0) {
                std::memcpy(x.data(), payload, count * sizeof(uint32_t));
                std::memcpy(y.data(), payload + count * sizeof(uint32_t), count * sizeof(uint32_t));
                std::memcpy(z.data(), payload + 2 * count * sizeof(uint32_t), count * sizeof(uint32_t));
            }
            fuzzMorton3dTyped<uint32_t>(
                    ZS_morton3dEncode32, ZS_morton3dDecode32,
                    ZS_morton3dEncodeInterleaved32, ZS_morton3dDecodeInterleaved32,
                    x.data(), y.data(), z.data(), count);
            break;
        }
        case 3: { // uint64_t
            size_t const count = payloadSize / (3 * sizeof(uint64_t));
            std::vector<uint64_t> x(count), y(count), z(count);
            if (count > 0) {
                std::memcpy(x.data(), payload, count * sizeof(uint64_t));
                std::memcpy(y.data(), payload + count * sizeof(uint64_t), count * sizeof(uint64_t));
                std::memcpy(z.data(), payload + 2 * count * sizeof(uint64_t), count * sizeof(uint64_t));
            }
            fuzzMorton3dTyped<uint64_t>(
                    ZS_morton3dEncode64, ZS_morton3dDecode64,
                    ZS_morton3dEncodeInterleaved64, ZS_morton3dDecodeInterleaved64,
                    x.data(), y.data(), z.data(), count);
            break;
        }
    }

    return 0;
}

#if defined(STANDALONE_FUZZER)
int main()
{
    uint8_t sampleData[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        1, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0,
        2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
    };
    LLVMFuzzerTestOneInput(sampleData, sizeof(sampleData));
    LLVMFuzzerTestOneInput(nullptr, 0);
    return 0;
}
#endif
