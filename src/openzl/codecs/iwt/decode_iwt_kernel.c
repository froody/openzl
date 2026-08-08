// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "decode_iwt_kernel.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static inline int64_t floor_div2(int64_t v)
{
    return v >> 1;
}

static inline int64_t floor_div4(int64_t v)
{
    return v >> 2;
}

#define DEFINE_IWT_DECODE(BITS, TYPE, STYPE)                                   \
void ZS_iwtDecode##BITS(TYPE* dst, TYPE const* srcApprox, TYPE const* srcDetail, size_t nbElts) \
{                                                                              \
    if (nbElts == 0) return;                                                   \
    size_t const nbApprox = (nbElts + 1) / 2;                                  \
    size_t const nbDetail = nbElts / 2;                                        \
    if (nbDetail == 0) {                                                       \
        dst[0] = srcApprox[0];                                                 \
        return;                                                                \
    }                                                                          \
    for (size_t n = 0; n < nbApprox; ++n) {                                    \
        STYPE left_d, right_d;                                                 \
        if (n == 0) {                                                          \
            left_d = (STYPE)srcDetail[0];                                      \
            right_d = (STYPE)srcDetail[0];                                     \
        } else {                                                               \
            left_d = (STYPE)srcDetail[n - 1];                                  \
            right_d = (n < nbDetail) ? (STYPE)srcDetail[n] : left_d;            \
        }                                                                      \
        STYPE upd = (STYPE)floor_div4((int64_t)((uint64_t)left_d + (uint64_t)right_d + 2)); \
        dst[2 * n] = (TYPE)((uint64_t)srcApprox[n] - (uint64_t)upd);           \
    }                                                                          \
    for (size_t n = 0; n < nbDetail; ++n) {                                    \
        STYPE left = (STYPE)dst[2 * n];                                        \
        STYPE right = (2 * n + 2 < nbElts) ? (STYPE)dst[2 * n + 2] : left;      \
        STYPE pred = (STYPE)floor_div2((int64_t)((uint64_t)left + (uint64_t)right)); \
        dst[2 * n + 1] = (TYPE)((uint64_t)srcDetail[n] + (uint64_t)pred);       \
    }                                                                          \
}

DEFINE_IWT_DECODE(8,  uint8_t,  int16_t)
DEFINE_IWT_DECODE(16, uint16_t, int32_t)
DEFINE_IWT_DECODE(32, uint32_t, int64_t)
DEFINE_IWT_DECODE(64, uint64_t, int64_t)

void ZS_iwtDecode(
        void* dst,
        void const* srcApprox,
        void const* srcDetail,
        size_t nbElts,
        size_t eltWidth)
{
    if (nbElts == 0) return;
    switch (eltWidth) {
        case 1:
            ZS_iwtDecode8((uint8_t*)dst, (uint8_t const*)srcApprox, (uint8_t const*)srcDetail, nbElts);
            break;
        case 2:
            ZS_iwtDecode16((uint16_t*)dst, (uint16_t const*)srcApprox, (uint16_t const*)srcDetail, nbElts);
            break;
        case 4:
            ZS_iwtDecode32((uint32_t*)dst, (uint32_t const*)srcApprox, (uint32_t const*)srcDetail, nbElts);
            break;
        case 8:
            ZS_iwtDecode64((uint64_t*)dst, (uint64_t const*)srcApprox, (uint64_t const*)srcDetail, nbElts);
            break;
        default:
            break;
    }
}
