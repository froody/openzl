// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/pcodec/decode_pcodec_kernel.h"
#include <assert.h>

void ZS_pcodecDecode(
        void* dst,
        const void* latents,
        const void* residuals,
        size_t numElts,
        size_t eltWidth,
        uint64_t baseOffset,
        uint64_t multiplier)
{
    if (multiplier == 0) {
        multiplier = 1;
    }
    switch (eltWidth) {
        case 1: {
            uint8_t* x = (uint8_t*)dst;
            const uint8_t* k = (const uint8_t*)latents;
            const uint8_t* r = (const uint8_t*)residuals;
            uint8_t base = (uint8_t)baseOffset;
            uint8_t mult = (uint8_t)multiplier;
            for (size_t i = 0; i < numElts; ++i) {
                x[i] = base + k[i] * mult + r[i];
            }
            break;
        }
        case 2: {
            uint16_t* x = (uint16_t*)dst;
            const uint16_t* k = (const uint16_t*)latents;
            const uint16_t* r = (const uint16_t*)residuals;
            uint16_t base = (uint16_t)baseOffset;
            uint16_t mult = (uint16_t)multiplier;
            for (size_t i = 0; i < numElts; ++i) {
                x[i] = base + k[i] * mult + r[i];
            }
            break;
        }
        case 4: {
            uint32_t* x = (uint32_t*)dst;
            const uint32_t* k = (const uint32_t*)latents;
            const uint32_t* r = (const uint32_t*)residuals;
            uint32_t base = (uint32_t)baseOffset;
            uint32_t mult = (uint32_t)multiplier;
            for (size_t i = 0; i < numElts; ++i) {
                x[i] = base + k[i] * mult + r[i];
            }
            break;
        }
        case 8: {
            uint64_t* x = (uint64_t*)dst;
            const uint64_t* k = (const uint64_t*)latents;
            const uint64_t* r = (const uint64_t*)residuals;
            uint64_t base = baseOffset;
            uint64_t mult = multiplier;
            for (size_t i = 0; i < numElts; ++i) {
                x[i] = base + k[i] * mult + r[i];
            }
            break;
        }
        default:
            assert(0);
    }
}
