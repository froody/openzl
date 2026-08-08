// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "decode_bitplane_split_kernel.h"
#include "encode_bitplane_split_kernel.h"

#include <stddef.h>
#include <stdint.h>

static inline uint64_t loadVal(void const* src, size_t index, size_t width)
{
    switch (width) {
        case 1:
            return ((uint8_t const*)src)[index];
        case 2:
            return ((uint16_t const*)src)[index];
        case 4:
            return ((uint32_t const*)src)[index];
        case 8:
            return ((uint64_t const*)src)[index];
        default:
            return 0;
    }
}

void ZS_bitplaneSplitDecode8(
        uint8_t* dst,
        void const* srcMsb,
        void const* srcLsb,
        size_t nbElts,
        uint8_t msbBits)
{
    if (nbElts == 0 || dst == NULL || srcMsb == NULL || srcLsb == NULL) return;
    if (msbBits == 0 || msbBits >= 8) msbBits = 4;
    uint8_t lsbBits = (uint8_t)(8 - msbBits);
    size_t msbWidth = ZS_bitplaneSplit_getMsbWidth(1, msbBits);
    size_t lsbWidth = ZS_bitplaneSplit_getLsbWidth(1, msbBits);

    for (size_t i = 0; i < nbElts; ++i) {
        uint64_t msbVal = loadVal(srcMsb, i, msbWidth);
        uint64_t lsbVal = loadVal(srcLsb, i, lsbWidth);
        dst[i] = (uint8_t)(((uint8_t)msbVal << lsbBits) | (uint8_t)lsbVal);
    }
}

void ZS_bitplaneSplitDecode16(
        uint16_t* dst,
        void const* srcMsb,
        void const* srcLsb,
        size_t nbElts,
        uint8_t msbBits)
{
    if (nbElts == 0 || dst == NULL || srcMsb == NULL || srcLsb == NULL) return;
    if (msbBits == 0 || msbBits >= 16) msbBits = 8;
    uint8_t lsbBits = (uint8_t)(16 - msbBits);
    size_t msbWidth = ZS_bitplaneSplit_getMsbWidth(2, msbBits);
    size_t lsbWidth = ZS_bitplaneSplit_getLsbWidth(2, msbBits);

    for (size_t i = 0; i < nbElts; ++i) {
        uint64_t msbVal = loadVal(srcMsb, i, msbWidth);
        uint64_t lsbVal = loadVal(srcLsb, i, lsbWidth);
        dst[i] = (uint16_t)(((uint16_t)msbVal << lsbBits) | (uint16_t)lsbVal);
    }
}

void ZS_bitplaneSplitDecode32(
        uint32_t* dst,
        void const* srcMsb,
        void const* srcLsb,
        size_t nbElts,
        uint8_t msbBits)
{
    if (nbElts == 0 || dst == NULL || srcMsb == NULL || srcLsb == NULL) return;
    if (msbBits == 0 || msbBits >= 32) msbBits = 16;
    uint8_t lsbBits = (uint8_t)(32 - msbBits);
    size_t msbWidth = ZS_bitplaneSplit_getMsbWidth(4, msbBits);
    size_t lsbWidth = ZS_bitplaneSplit_getLsbWidth(4, msbBits);

    for (size_t i = 0; i < nbElts; ++i) {
        uint64_t msbVal = loadVal(srcMsb, i, msbWidth);
        uint64_t lsbVal = loadVal(srcLsb, i, lsbWidth);
        dst[i] = (uint32_t)(((uint32_t)msbVal << lsbBits) | (uint32_t)lsbVal);
    }
}

void ZS_bitplaneSplitDecode64(
        uint64_t* dst,
        void const* srcMsb,
        void const* srcLsb,
        size_t nbElts,
        uint8_t msbBits)
{
    if (nbElts == 0 || dst == NULL || srcMsb == NULL || srcLsb == NULL) return;
    if (msbBits == 0 || msbBits >= 64) msbBits = 32;
    uint8_t lsbBits = (uint8_t)(64 - msbBits);
    size_t msbWidth = ZS_bitplaneSplit_getMsbWidth(8, msbBits);
    size_t lsbWidth = ZS_bitplaneSplit_getLsbWidth(8, msbBits);

    for (size_t i = 0; i < nbElts; ++i) {
        uint64_t msbVal = loadVal(srcMsb, i, msbWidth);
        uint64_t lsbVal = loadVal(srcLsb, i, lsbWidth);
        dst[i] = (uint64_t)(((uint64_t)msbVal << lsbBits) | (uint64_t)lsbVal);
    }
}

void ZS_bitplaneSplitDecode(
        void* dst,
        void const* srcMsb,
        void const* srcLsb,
        size_t nbElts,
        size_t eltWidth,
        uint8_t msbBits)
{
    if (nbElts == 0 || dst == NULL) return;
    switch (eltWidth) {
        case 1:
            ZS_bitplaneSplitDecode8((uint8_t*)dst, srcMsb, srcLsb, nbElts, msbBits);
            break;
        case 2:
            ZS_bitplaneSplitDecode16((uint16_t*)dst, srcMsb, srcLsb, nbElts, msbBits);
            break;
        case 4:
            ZS_bitplaneSplitDecode32((uint32_t*)dst, srcMsb, srcLsb, nbElts, msbBits);
            break;
        case 8:
            ZS_bitplaneSplitDecode64((uint64_t*)dst, srcMsb, srcLsb, nbElts, msbBits);
            break;
        default:
            break;
    }
}
