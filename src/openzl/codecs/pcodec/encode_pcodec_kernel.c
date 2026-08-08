// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/pcodec/encode_pcodec_kernel.h"
#include <assert.h>
#include <stdint.h>

void ZS_pcodecEncode(
        void* latentsOut,
        void* residualsOut,
        const void* src,
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
            uint8_t* k = (uint8_t*)latentsOut;
            uint8_t* r = (uint8_t*)residualsOut;
            const uint8_t* x = (const uint8_t*)src;
            uint8_t base = (uint8_t)baseOffset;
            uint8_t mult = (uint8_t)multiplier;
            for (size_t i = 0; i < numElts; ++i) {
                uint8_t val = x[i] - base;
                k[i] = val / mult;
                r[i] = val % mult;
            }
            break;
        }
        case 2: {
            uint16_t* k = (uint16_t*)latentsOut;
            uint16_t* r = (uint16_t*)residualsOut;
            const uint16_t* x = (const uint16_t*)src;
            uint16_t base = (uint16_t)baseOffset;
            uint16_t mult = (uint16_t)multiplier;
            for (size_t i = 0; i < numElts; ++i) {
                uint16_t val = x[i] - base;
                k[i] = val / mult;
                r[i] = val % mult;
            }
            break;
        }
        case 4: {
            uint32_t* k = (uint32_t*)latentsOut;
            uint32_t* r = (uint32_t*)residualsOut;
            const uint32_t* x = (const uint32_t*)src;
            uint32_t base = (uint32_t)baseOffset;
            uint32_t mult = (uint32_t)multiplier;
            for (size_t i = 0; i < numElts; ++i) {
                uint32_t val = x[i] - base;
                k[i] = val / mult;
                r[i] = val % mult;
            }
            break;
        }
        case 8: {
            uint64_t* k = (uint64_t*)latentsOut;
            uint64_t* r = (uint64_t*)residualsOut;
            const uint64_t* x = (const uint64_t*)src;
            uint64_t base = baseOffset;
            uint64_t mult = multiplier;
            for (size_t i = 0; i < numElts; ++i) {
                uint64_t val = x[i] - base;
                k[i] = val / mult;
                r[i] = val % mult;
            }
            break;
        }
        default:
            assert(0);
    }
}
