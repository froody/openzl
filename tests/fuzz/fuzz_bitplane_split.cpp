// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "openzl/codecs/bitplane_split/decode_bitplane_split_kernel.h"
#include "openzl/codecs/bitplane_split/encode_bitplane_split_kernel.h"

namespace {

template <typename T>
void fuzzBitplaneSplitTyped(
        void (*encodeFn)(void*, void*, const T*, size_t, uint8_t),
        void (*decodeFn)(T*, const void*, const void*, size_t, uint8_t),
        const T* input,
        size_t count,
        uint8_t msbBits)
{
    if (count == 0) {
        encodeFn(nullptr, nullptr, nullptr, 0, msbBits);
        decodeFn(nullptr, nullptr, nullptr, 0, msbBits);
        ZS_bitplaneSplitEncode(nullptr, nullptr, nullptr, 0, sizeof(T), msbBits);
        ZS_bitplaneSplitDecode(nullptr, nullptr, nullptr, 0, sizeof(T), msbBits);
        return;
    }

    size_t const msbWidth = ZS_bitplaneSplit_getMsbWidth(sizeof(T), msbBits);
    size_t const lsbWidth = ZS_bitplaneSplit_getLsbWidth(sizeof(T), msbBits);

    std::vector<uint8_t> msbBuf(count * msbWidth);
    std::vector<uint8_t> lsbBuf(count * lsbWidth);
    std::vector<T> output(count, static_cast<T>(0));

    // 1. Typed kernel encode/decode roundtrip
    encodeFn(msbBuf.data(), lsbBuf.data(), input, count, msbBits);
    decodeFn(output.data(), msbBuf.data(), lsbBuf.data(), count, msbBits);

    if (std::memcmp(output.data(), input, count * sizeof(T)) != 0) {
        std::abort(); // Roundtrip failure trigger
    }

    // 2. Generic ZS_bitplaneSplitEncode / ZS_bitplaneSplitDecode roundtrip
    std::fill(msbBuf.begin(), msbBuf.end(), 0);
    std::fill(lsbBuf.begin(), lsbBuf.end(), 0);
    std::fill(output.begin(), output.end(), static_cast<T>(0));

    ZS_bitplaneSplitEncode(msbBuf.data(), lsbBuf.data(), input, count, sizeof(T), msbBits);
    ZS_bitplaneSplitDecode(output.data(), msbBuf.data(), lsbBuf.data(), count, sizeof(T), msbBits);

    if (std::memcmp(output.data(), input, count * sizeof(T)) != 0) {
        std::abort(); // Roundtrip failure trigger
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) {
        fuzzBitplaneSplitTyped<uint8_t>(ZS_bitplaneSplitEncode8, ZS_bitplaneSplitDecode8, nullptr, 0, 4);
        fuzzBitplaneSplitTyped<uint16_t>(ZS_bitplaneSplitEncode16, ZS_bitplaneSplitDecode16, nullptr, 0, 8);
        fuzzBitplaneSplitTyped<uint32_t>(ZS_bitplaneSplitEncode32, ZS_bitplaneSplitDecode32, nullptr, 0, 16);
        fuzzBitplaneSplitTyped<uint64_t>(ZS_bitplaneSplitEncode64, ZS_bitplaneSplitDecode64, nullptr, 0, 32);
        return 0;
    }

    uint8_t const mode = data[0] % 4;
    uint8_t const rawMsbBits = data[1];
    const uint8_t* payload = data + 2;
    size_t const payloadSize = (size >= 2) ? (size - 2) : 0;

    switch (mode) {
        case 0: { // uint8_t (1 byte)
            size_t const count = payloadSize / sizeof(uint8_t);
            std::vector<uint8_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint8_t));
            }
            uint8_t msbBits = rawMsbBits % 8;
            fuzzBitplaneSplitTyped<uint8_t>(
                    ZS_bitplaneSplitEncode8,
                    ZS_bitplaneSplitDecode8,
                    vec.data(),
                    count,
                    msbBits);
            break;
        }
        case 1: { // uint16_t (2 bytes)
            size_t const count = payloadSize / sizeof(uint16_t);
            std::vector<uint16_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint16_t));
            }
            uint8_t msbBits = rawMsbBits % 16;
            fuzzBitplaneSplitTyped<uint16_t>(
                    ZS_bitplaneSplitEncode16,
                    ZS_bitplaneSplitDecode16,
                    vec.data(),
                    count,
                    msbBits);
            break;
        }
        case 2: { // uint32_t / float (4 bytes)
            size_t const count = payloadSize / sizeof(uint32_t);
            std::vector<uint32_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint32_t));
            }
            uint8_t msbBits = rawMsbBits % 32;
            fuzzBitplaneSplitTyped<uint32_t>(
                    ZS_bitplaneSplitEncode32,
                    ZS_bitplaneSplitDecode32,
                    vec.data(),
                    count,
                    msbBits);
            break;
        }
        case 3: { // uint64_t / double (8 bytes)
            size_t const count = payloadSize / sizeof(uint64_t);
            std::vector<uint64_t> vec(count);
            if (count > 0) {
                std::memcpy(vec.data(), payload, count * sizeof(uint64_t));
            }
            uint8_t msbBits = rawMsbBits % 64;
            fuzzBitplaneSplitTyped<uint64_t>(
                    ZS_bitplaneSplitEncode64,
                    ZS_bitplaneSplitDecode64,
                    vec.data(),
                    count,
                    msbBits);
            break;
        }
    }

    return 0;
}

#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION) && !defined(LIBFUZZER)
int main()
{
    // Standalone runner for cmake tests / ctest / test execution
    // Run edge cases and synthetic inputs
    std::vector<uint8_t> testData = {
        0, 4, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        1, 8, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        2, 16, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        3, 32, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0
    };

    LLVMFuzzerTestOneInput(nullptr, 0);
    LLVMFuzzerTestOneInput(testData.data(), testData.size());

    // Run multiple random patterns
    std::vector<uint8_t> rndBuf(1024);
    for (size_t i = 0; i < rndBuf.size(); ++i) {
        rndBuf[i] = static_cast<uint8_t>((i * 131 + 17) % 256);
    }
    for (size_t len = 1; len <= rndBuf.size(); len += 13) {
        LLVMFuzzerTestOneInput(rndBuf.data(), len);
    }

    return 0;
}
#endif
