// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "openzl/codecs/iwt/decode_iwt_kernel.h"
#include "openzl/codecs/iwt/encode_iwt_kernel.h"

namespace {

template <typename T>
void fuzzIwtTyped(
        void (*encodeFn)(T*, T*, const T*, size_t),
        void (*decodeFn)(T*, const T*, const T*, size_t),
        const T* input,
        size_t count)
{
    if (count == 0) {
        encodeFn(nullptr, nullptr, nullptr, 0);
        decodeFn(nullptr, nullptr, nullptr, 0);
        ZS_iwtEncode(nullptr, nullptr, nullptr, 0, sizeof(T));
        ZS_iwtDecode(nullptr, nullptr, nullptr, 0, sizeof(T));
        return;
    }

    size_t const nbApprox = (count + 1) / 2;
    size_t const nbDetail = count / 2;

    std::vector<T> approx(nbApprox);
    std::vector<T> detail(nbDetail);
    std::vector<T> output(count, static_cast<T>(0));

    // 1. Direct typed kernel encode/decode roundtrip
    encodeFn(approx.data(), detail.data(), input, count);
    decodeFn(output.data(), approx.data(), detail.data(), count);

    if (memcmp(output.data(), input, count * sizeof(T)) != 0) {
        abort(); // Fuzzer crash trigger on roundtrip mismatch
    }

    // 2. Generic ZS_iwtEncode / ZS_iwtDecode roundtrip
    std::fill(approx.begin(), approx.end(), static_cast<T>(0));
    std::fill(detail.begin(), detail.end(), static_cast<T>(0));
    std::fill(output.begin(), output.end(), static_cast<T>(0));

    ZS_iwtEncode(approx.data(), detail.data(), input, count, sizeof(T));
    ZS_iwtDecode(output.data(), approx.data(), detail.data(), count, sizeof(T));

    if (memcmp(output.data(), input, count * sizeof(T)) != 0) {
        abort(); // Fuzzer crash trigger on roundtrip mismatch
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) {
        fuzzIwtTyped<uint8_t>(ZS_iwtEncode8, ZS_iwtDecode8, nullptr, 0);
        fuzzIwtTyped<uint16_t>(ZS_iwtEncode16, ZS_iwtDecode16, nullptr, 0);
        fuzzIwtTyped<uint32_t>(ZS_iwtEncode32, ZS_iwtDecode32, nullptr, 0);
        fuzzIwtTyped<uint64_t>(ZS_iwtEncode64, ZS_iwtDecode64, nullptr, 0);
        return 0;
    }

    // Use the first byte to pick the element width (1, 2, 4, or 8 bytes)
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
            fuzzIwtTyped<uint8_t>(
                    ZS_iwtEncode8,
                    ZS_iwtDecode8,
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
            fuzzIwtTyped<uint16_t>(
                    ZS_iwtEncode16,
                    ZS_iwtDecode16,
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
            fuzzIwtTyped<uint32_t>(
                    ZS_iwtEncode32,
                    ZS_iwtDecode32,
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
            fuzzIwtTyped<uint64_t>(
                    ZS_iwtEncode64,
                    ZS_iwtDecode64,
                    vec.data(),
                    count);
            break;
        }
    }

    return 0;
}
