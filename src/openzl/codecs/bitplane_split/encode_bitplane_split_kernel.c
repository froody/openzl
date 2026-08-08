// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "encode_bitplane_split_kernel.h"

#include <stddef.h>
#include <stdint.h>

size_t ZS_bitplaneSplit_getMsbWidth(size_t eltWidth, uint8_t msbBits)
{
    (void)eltWidth;
    if (msbBits <= 8) return 1;
    if (msbBits <= 16) return 2;
    if (msbBits <= 32) return 4;
    return 8;
}

size_t ZS_bitplaneSplit_getLsbWidth(size_t eltWidth, uint8_t msbBits)
{
    uint8_t totalBits = (uint8_t)(eltWidth * 8);
    uint8_t lsbBits = (msbBits < totalBits) ? (uint8_t)(totalBits - msbBits) : 0;
    if (lsbBits <= 8) return 1;
    if (lsbBits <= 16) return 2;
    if (lsbBits <= 32) return 4;
    return 8;
}

static inline void storeVal(void* dst, size_t index, uint64_t val, size_t width)
{
    switch (width) {
        case 1:
            ((uint8_t*)dst)[index] = (uint8_t)val;
            break;
        case 2:
            ((uint16_t*)dst)[index] = (uint16_t)val;
            break;
        case 4:
            ((uint32_t*)dst)[index] = (uint32_t)val;
            break;
        case 8:
            ((uint64_t*)dst)[index] = (uint64_t)val;
            break;
        default:
            break;
    }
}

void ZS_bitplaneSplitEncode8(
        void* dstMsb,
        void* dstLsb,
        uint8_t const* src,
        size_t nbElts,
        uint8_t msbBits)
{
    if (nbElts == 0 || src == NULL || dstMsb == NULL || dstLsb == NULL) return;
    if (msbBits == 0 || msbBits >= 8) msbBits = 4;
    uint8_t lsbBits = (uint8_t)(8 - msbBits);
    size_t msbWidth = ZS_bitplaneSplit_getMsbWidth(1, msbBits);
    size_t lsbWidth = ZS_bitplaneSplit_getLsbWidth(1, msbBits);
    uint8_t msbMask = (uint8_t)((1U << msbBits) - 1);
    uint8_t lsbMask = (uint8_t)((1U << lsbBits) - 1);

    for (size_t i = 0; i < nbElts; ++i) {
        uint8_t val = src[i];
        uint8_t msbVal = (uint8_t)((val >> lsbBits) & msbMask);
        uint8_t lsbVal = (uint8_t)(val & lsbMask);
        storeVal(dstMsb, i, msbVal, msbWidth);
        storeVal(dstLsb, i, lsbVal, lsbWidth);
    }
}

void ZS_bitplaneSplitEncode16(
        void* dstMsb,
        void* dstLsb,
        uint16_t const* src,
        size_t nbElts,
        uint8_t msbBits)
{
    if (nbElts == 0 || src == NULL || dstMsb == NULL || dstLsb == NULL) return;
    if (msbBits == 0 || msbBits >= 16) msbBits = 8;
    uint8_t lsbBits = (uint8_t)(16 - msbBits);
    size_t msbWidth = ZS_bitplaneSplit_getMsbWidth(2, msbBits);
    size_t lsbWidth = ZS_bitplaneSplit_getLsbWidth(2, msbBits);
    uint16_t msbMask = (uint16_t)((1U << msbBits) - 1);
    uint16_t lsbMask = (uint16_t)((1U << lsbBits) - 1);

    for (size_t i = 0; i < nbElts; ++i) {
        uint16_t val = src[i];
        uint16_t msbVal = (uint16_t)((val >> lsbBits) & msbMask);
        uint16_t lsbVal = (uint16_t)(val & lsbMask);
        storeVal(dstMsb, i, msbVal, msbWidth);
        storeVal(dstLsb, i, lsbVal, lsbWidth);
    }
}

void ZS_bitplaneSplitEncode32(
        void* dstMsb,
        void* dstLsb,
        uint32_t const* src,
        size_t nbElts,
        uint8_t msbBits)
{
    if (nbElts == 0 || src == NULL || dstMsb == NULL || dstLsb == NULL) return;
    if (msbBits == 0 || msbBits >= 32) msbBits = 16;
    uint8_t lsbBits = (uint8_t)(32 - msbBits);
    size_t msbWidth = ZS_bitplaneSplit_getMsbWidth(4, msbBits);
    size_t lsbWidth = ZS_bitplaneSplit_getLsbWidth(4, msbBits);
    uint32_t msbMask = (msbBits == 32) ? 0xFFFFFFFFU : (uint32_t)((1U << msbBits) - 1);
    uint32_t lsbMask = (lsbBits == 32) ? 0xFFFFFFFFU : (uint32_t)((1U << lsbBits) - 1);

    for (size_t i = 0; i < nbElts; ++i) {
        uint32_t val = src[i];
        uint32_t msbVal = (val >> lsbBits) & msbMask;
        uint32_t lsbVal = val & lsbMask;
        storeVal(dstMsb, i, msbVal, msbWidth);
        storeVal(dstLsb, i, lsbVal, lsbWidth);
    }
}

void ZS_bitplaneSplitEncode64(
        void* dstMsb,
        void* dstLsb,
        uint64_t const* src,
        size_t nbElts,
        uint8_t msbBits)
{
    if (nbElts == 0 || src == NULL || dstMsb == NULL || dstLsb == NULL) return;
    if (msbBits == 0 || msbBits >= 64) msbBits = 32;
    uint8_t lsbBits = (uint8_t)(64 - msbBits);
    size_t msbWidth = ZS_bitplaneSplit_getMsbWidth(8, msbBits);
    size_t lsbWidth = ZS_bitplaneSplit_getLsbWidth(8, msbBits);
    uint64_t msbMask = (msbBits == 64) ? 0xFFFFFFFFFFFFFFFFULL : (((uint64_t)1 << msbBits) - 1);
    uint64_t lsbMask = (lsbBits == 64) ? 0xFFFFFFFFFFFFFFFFULL : (((uint64_t)1 << lsbBits) - 1);

    for (size_t i = 0; i < nbElts; ++i) {
        uint64_t val = src[i];
        uint64_t msbVal = (val >> lsbBits) & msbMask;
        uint64_t lsbVal = val & lsbMask;
        storeVal(dstMsb, i, msbVal, msbWidth);
        storeVal(dstLsb, i, lsbVal, lsbWidth);
    }
}

void ZS_bitplaneSplitEncode(
        void* dstMsb,
        void* dstLsb,
        void const* src,
        size_t nbElts,
        size_t eltWidth,
        uint8_t msbBits)
{
    if (nbElts == 0 || src == NULL) return;
    switch (eltWidth) {
        case 1:
            ZS_bitplaneSplitEncode8(dstMsb, dstLsb, (uint8_t const*)src, nbElts, msbBits);
            break;
        case 2:
            ZS_bitplaneSplitEncode16(dstMsb, dstLsb, (uint16_t const*)src, nbElts, msbBits);
            break;
        case 4:
            ZS_bitplaneSplitEncode32(dstMsb, dstLsb, (uint32_t const*)src, nbElts, msbBits);
            break;
        case 8:
            ZS_bitplaneSplitEncode64(dstMsb, dstLsb, (uint64_t const*)src, nbElts, msbBits);
            break;
        default:
            break;
    }
}
