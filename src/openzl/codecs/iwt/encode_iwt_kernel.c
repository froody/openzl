// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "encode_iwt_kernel.h"
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

#define DEFINE_IWT_ENCODE(BITS, TYPE, STYPE)                                   \
void ZS_iwtEncode##BITS(TYPE* dstApprox, TYPE* dstDetail, TYPE const* src, size_t nbElts) \
{                                                                              \
    if (nbElts == 0) return;                                                   \
    size_t const nbApprox = (nbElts + 1) / 2;                                  \
    size_t const nbDetail = nbElts / 2;                                        \
    if (nbDetail == 0) {                                                       \
        dstApprox[0] = src[0];                                                 \
        return;                                                                \
    }                                                                          \
    for (size_t n = 0; n < nbDetail; ++n) {                                    \
        STYPE left = (STYPE)src[2 * n];                                        \
        STYPE right = (2 * n + 2 < nbElts) ? (STYPE)src[2 * n + 2] : left;      \
        STYPE pred = (STYPE)floor_div2((int64_t)((uint64_t)left + (uint64_t)right)); \
        dstDetail[n] = (TYPE)((uint64_t)src[2 * n + 1] - (uint64_t)pred);      \
    }                                                                          \
    for (size_t n = 0; n < nbApprox; ++n) {                                    \
        STYPE left_d, right_d;                                                 \
        if (n == 0) {                                                          \
            left_d = (STYPE)dstDetail[0];                                      \
            right_d = (STYPE)dstDetail[0];                                     \
        } else {                                                               \
            left_d = (STYPE)dstDetail[n - 1];                                  \
            right_d = (n < nbDetail) ? (STYPE)dstDetail[n] : left_d;            \
        }                                                                      \
        STYPE upd = (STYPE)floor_div4((int64_t)((uint64_t)left_d + (uint64_t)right_d + 2)); \
        dstApprox[n] = (TYPE)((uint64_t)src[2 * n] + (uint64_t)upd);           \
    }                                                                          \
}

DEFINE_IWT_ENCODE(8,  uint8_t,  int16_t)
DEFINE_IWT_ENCODE(16, uint16_t, int32_t)
DEFINE_IWT_ENCODE(32, uint32_t, int64_t)
DEFINE_IWT_ENCODE(64, uint64_t, int64_t)

void ZS_iwtEncode(
        void* dstApprox,
        void* dstDetail,
        void const* src,
        size_t nbElts,
        size_t eltWidth)
{
    if (nbElts == 0) return;
    switch (eltWidth) {
        case 1:
            ZS_iwtEncode8((uint8_t*)dstApprox, (uint8_t*)dstDetail, (uint8_t const*)src, nbElts);
            break;
        case 2:
            ZS_iwtEncode16((uint16_t*)dstApprox, (uint16_t*)dstDetail, (uint16_t const*)src, nbElts);
            break;
        case 4:
            ZS_iwtEncode32((uint32_t*)dstApprox, (uint32_t*)dstDetail, (uint32_t const*)src, nbElts);
            break;
        case 8:
            ZS_iwtEncode64((uint64_t*)dstApprox, (uint64_t*)dstDetail, (uint64_t const*)src, nbElts);
            break;
        default:
            break;
    }
}
