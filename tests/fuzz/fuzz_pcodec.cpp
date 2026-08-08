// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "openzl/codecs/pcodec/decode_pcodec_kernel.h"
#include "openzl/codecs/pcodec/encode_pcodec_kernel.h"

namespace {

template <typename T>
void fuzzPcodecTyped(const T* input, size_t count, uint64_t baseOffset, uint64_t multiplier)
{
    if (count == 0) {
        ZS_pcodecEncode(nullptr, nullptr, nullptr, 0, sizeof(T), baseOffset, multiplier);
        ZS_pcodecDecode(nullptr, nullptr, nullptr, 0, sizeof(T), baseOffset, multiplier);
        return;
    }

    std::vector<T> latents(count);
    std::vector<T> residuals(count);
    std::vector<T> output(count, static_cast<T>(0));

    // 1. Kernel encode/decode roundtrip
    ZS_pcodecEncode(latents.data(), residuals.data(), input, count, sizeof(T), baseOffset, multiplier);
    ZS_pcodecDecode(output.data(), latents.data(), residuals.data(), count, sizeof(T), baseOffset, multiplier);

    if (memcmp(output.data(), input, count * sizeof(T)) != 0) {
        abort(); // Fuzzer crash trigger on roundtrip mismatch
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < 10) {
        // Edge cases with empty input
        fuzzPcodecTyped<uint8_t>(nullptr, 0, 0, 1);
        fuzzPcodecTyped<uint16_t>(nullptr, 0, 0, 1);
        fuzzPcodecTyped<uint32_t>(nullptr, 0, 0, 1);
        fuzzPcodecTyped<uint64_t>(nullptr, 0, 0, 1);
        return 0;
    }

    uint8_t const mode = data[0] % 4;
    uint8_t const multByte = data[1];
    uint64_t const multiplier = (multByte == 0) ? 1 : multByte;

    const uint8_t* payload = data + 2;
    size_t const payloadSize = size - 2;

    switch (mode) {
        case 0: { // uint8_t (1 byte)
            size_t const count = payloadSize / sizeof(uint8_t);
            std::vector<uint8_t> vec(count);
            std::memcpy(vec.data(), payload, count * sizeof(uint8_t));

            uint8_t baseOffset = vec[0];
            for (size_t i = 1; i < count; ++i) {
                if (vec[i] < baseOffset) baseOffset = vec[i];
            }

            fuzzPcodecTyped<uint8_t>(vec.data(), count, baseOffset, multiplier);
            break;
        }
        case 1: { // uint16_t (2 bytes)
            size_t const count = payloadSize / sizeof(uint16_t);
            std::vector<uint16_t> vec(count);
            std::memcpy(vec.data(), payload, count * sizeof(uint16_t));

            uint16_t baseOffset = vec[0];
            for (size_t i = 1; i < count; ++i) {
                if (vec[i] < baseOffset) baseOffset = vec[i];
            }

            fuzzPcodecTyped<uint16_t>(vec.data(), count, baseOffset, multiplier);
            break;
        }
        case 2: { // uint32_t (4 bytes)
            size_t const count = payloadSize / sizeof(uint32_t);
            std::vector<uint32_t> vec(count);
            std::memcpy(vec.data(), payload, count * sizeof(uint32_t));

            uint32_t baseOffset = vec[0];
            for (size_t i = 1; i < count; ++i) {
                if (vec[i] < baseOffset) baseOffset = vec[i];
            }

            fuzzPcodecTyped<uint32_t>(vec.data(), count, baseOffset, multiplier);
            break;
        }
        case 3: { // uint64_t (8 bytes)
            size_t const count = payloadSize / sizeof(uint64_t);
            std::vector<uint64_t> vec(count);
            std::memcpy(vec.data(), payload, count * sizeof(uint64_t));

            uint64_t baseOffset = vec[0];
            for (size_t i = 1; i < count; ++i) {
                if (vec[i] < baseOffset) baseOffset = vec[i];
            }

            fuzzPcodecTyped<uint64_t>(vec.data(), count, baseOffset, multiplier);
            break;
        }
    }

    return 0;
}
