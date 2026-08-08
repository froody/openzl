// Copyright (c) Meta Platforms, Inc. and affiliates.
// Coverage-guided LLVM libFuzzer target for SIMD Delta & Zigzag Kernels.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include "openzl/codecs/delta/decode_delta_kernel.h"
#include "openzl/codecs/delta/encode_delta_kernel.h"
#include "openzl/codecs/zigzag/decode_zigzag_kernel.h"
#include "openzl/codecs/zigzag/encode_zigzag_kernel.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size == 0) {
        return 0;
    }

    // ------------------------------------------------------------------------
    // 1. Delta 8-bit Round-Trip Test
    // ------------------------------------------------------------------------
    {
        size_t const nelts = size;
        uint8_t first      = 0;
        std::vector<uint8_t> deltas(nelts > 0 ? nelts - 1 : 0);
        std::vector<uint8_t> decoded(nelts);

        ZS_deltaEncode8(&first, deltas.data(), data, nelts);
        ZS_deltaDecode8(decoded.data(), first, deltas.data(), nelts);

        assert(memcmp(data, decoded.data(), nelts) == 0);
    }

    // ------------------------------------------------------------------------
    // 2. Delta 16-bit Round-Trip Test
    // ------------------------------------------------------------------------
    if (size >= sizeof(uint16_t)) {
        size_t const nelts        = size / sizeof(uint16_t);
        uint16_t const* src16     = reinterpret_cast<uint16_t const*>(data);
        uint16_t first            = 0;
        std::vector<uint16_t> deltas(nelts > 0 ? nelts - 1 : 0);
        std::vector<uint16_t> decoded(nelts);

        ZS_deltaEncode16(&first, deltas.data(), src16, nelts);
        ZS_deltaDecode16(decoded.data(), first, deltas.data(), nelts);

        assert(memcmp(src16, decoded.data(), nelts * sizeof(uint16_t)) == 0);
    }

    // ------------------------------------------------------------------------
    // 3. Delta 32-bit Round-Trip Test
    // ------------------------------------------------------------------------
    if (size >= sizeof(uint32_t)) {
        size_t const nelts        = size / sizeof(uint32_t);
        uint32_t const* src32     = reinterpret_cast<uint32_t const*>(data);
        uint32_t first            = 0;
        std::vector<uint32_t> deltas(nelts > 0 ? nelts - 1 : 0);
        std::vector<uint32_t> decoded(nelts);

        ZS_deltaEncode32(&first, deltas.data(), src32, nelts);
        ZS_deltaDecode32(decoded.data(), first, deltas.data(), nelts);

        assert(memcmp(src32, decoded.data(), nelts * sizeof(uint32_t)) == 0);
    }

    // ------------------------------------------------------------------------
    // 4. Zigzag 8-bit Round-Trip Test
    // ------------------------------------------------------------------------
    {
        size_t const nelts = size;
        int8_t const* src8 = reinterpret_cast<int8_t const*>(data);
        std::vector<uint8_t> encoded(nelts);
        std::vector<int8_t> decoded(nelts);

        ZL_zigzagEncode8(encoded.data(), src8, nelts);
        ZL_zigzagDecode8(decoded.data(), encoded.data(), nelts);

        assert(memcmp(src8, decoded.data(), nelts) == 0);
    }

    // ------------------------------------------------------------------------
    // 5. Zigzag 16-bit Round-Trip Test
    // ------------------------------------------------------------------------
    if (size >= sizeof(uint16_t)) {
        size_t const nelts   = size / sizeof(uint16_t);
        int16_t const* src16 = reinterpret_cast<int16_t const*>(data);
        std::vector<uint16_t> encoded(nelts);
        std::vector<int16_t> decoded(nelts);

        ZL_zigzagEncode16(encoded.data(), src16, nelts);
        ZL_zigzagDecode16(decoded.data(), encoded.data(), nelts);

        assert(memcmp(src16, decoded.data(), nelts * sizeof(int16_t)) == 0);
    }

    // ------------------------------------------------------------------------
    // 6. Zigzag 32-bit Round-Trip Test
    // ------------------------------------------------------------------------
    if (size >= sizeof(uint32_t)) {
        size_t const nelts   = size / sizeof(uint32_t);
        int32_t const* src32 = reinterpret_cast<int32_t const*>(data);
        std::vector<uint32_t> encoded(nelts);
        std::vector<int32_t> decoded(nelts);

        ZL_zigzagEncode32(encoded.data(), src32, nelts);
        ZL_zigzagDecode32(decoded.data(), encoded.data(), nelts);

        assert(memcmp(src32, decoded.data(), nelts * sizeof(int32_t)) == 0);
    }

    // ------------------------------------------------------------------------
    // 7. Zigzag 64-bit Round-Trip Test
    // ------------------------------------------------------------------------
    if (size >= sizeof(uint64_t)) {
        size_t const nelts   = size / sizeof(uint64_t);
        int64_t const* src64 = reinterpret_cast<int64_t const*>(data);
        std::vector<uint64_t> encoded(nelts);
        std::vector<int64_t> decoded(nelts);

        ZL_zigzagEncode64(encoded.data(), src64, nelts);
        ZL_zigzagDecode64(decoded.data(), encoded.data(), nelts);

        assert(memcmp(src64, decoded.data(), nelts * sizeof(int64_t)) == 0);
    }

    return 0;
}
