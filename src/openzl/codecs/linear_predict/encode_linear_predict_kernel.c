// Copyright (c) Meta Platforms, Inc. and affiliates.

#include <assert.h>
#include "encode_linear_predict_kernel.h"

static inline uint8_t computePrediction8(
        const uint8_t* p,
        size_t i,
        size_t stride,
        uint8_t mode)
{
    uint8_t const p_prev = (i > 0) ? p[i - 1] : 0;
    uint8_t const p_stride = (i >= stride) ? p[i - stride] : 0;
    uint8_t const p_stride_prev = (i > stride) ? p[i - stride - 1] : 0;
    uint8_t const p_2stride = (i >= 2 * stride) ? p[i - 2 * stride] : 0;

    switch (mode) {
        case 0:
            return (uint8_t)(p_prev + p_stride - p_stride_prev);
        case 1:
            return p_stride;
        case 2:
            return (uint8_t)(2 * p_stride - p_2stride);
        case 3:
            return (uint8_t)(((unsigned int)p_prev + p_stride) / 2);
        default:
            return (uint8_t)(p_prev + p_stride - p_stride_prev);
    }
}

static inline uint16_t computePrediction16(
        const uint16_t* p,
        size_t i,
        size_t stride,
        uint8_t mode)
{
    uint16_t const p_prev = (i > 0) ? p[i - 1] : 0;
    uint16_t const p_stride = (i >= stride) ? p[i - stride] : 0;
    uint16_t const p_stride_prev = (i > stride) ? p[i - stride - 1] : 0;
    uint16_t const p_2stride = (i >= 2 * stride) ? p[i - 2 * stride] : 0;

    switch (mode) {
        case 0:
            return (uint16_t)(p_prev + p_stride - p_stride_prev);
        case 1:
            return p_stride;
        case 2:
            return (uint16_t)(2 * p_stride - p_2stride);
        case 3:
            return (uint16_t)(((unsigned int)p_prev + p_stride) / 2);
        default:
            return (uint16_t)(p_prev + p_stride - p_stride_prev);
    }
}

static inline uint32_t computePrediction32(
        const uint32_t* p,
        size_t i,
        size_t stride,
        uint8_t mode)
{
    uint32_t const p_prev = (i > 0) ? p[i - 1] : 0;
    uint32_t const p_stride = (i >= stride) ? p[i - stride] : 0;
    uint32_t const p_stride_prev = (i > stride) ? p[i - stride - 1] : 0;
    uint32_t const p_2stride = (i >= 2 * stride) ? p[i - 2 * stride] : 0;

    switch (mode) {
        case 0:
            return (uint32_t)(p_prev + p_stride - p_stride_prev);
        case 1:
            return p_stride;
        case 2:
            return (uint32_t)(2 * (uint64_t)p_stride - p_2stride);
        case 3:
            return (uint32_t)(((uint64_t)p_prev + p_stride) / 2);
        default:
            return (uint32_t)(p_prev + p_stride - p_stride_prev);
    }
}

static inline uint64_t computePrediction64(
        const uint64_t* p,
        size_t i,
        size_t stride,
        uint8_t mode)
{
    uint64_t const p_prev = (i > 0) ? p[i - 1] : 0;
    uint64_t const p_stride = (i >= stride) ? p[i - stride] : 0;
    uint64_t const p_stride_prev = (i > stride) ? p[i - stride - 1] : 0;
    uint64_t const p_2stride = (i >= 2 * stride) ? p[i - 2 * stride] : 0;

    switch (mode) {
        case 0:
            return (uint64_t)(p_prev + p_stride - p_stride_prev);
        case 1:
            return p_stride;
        case 2:
            return (uint64_t)(2 * p_stride - p_2stride);
        case 3:
            return (p_prev & p_stride) + ((p_prev ^ p_stride) >> 1);
        default:
            return (uint64_t)(p_prev + p_stride - p_stride_prev);
    }
}

void ZS_linearPredictEncode8(
        uint8_t* dst,
        const uint8_t* src,
        size_t nbElts,
        size_t stride,
        uint8_t mode)
{
    if (nbElts == 0 || src == NULL || dst == NULL) {
        return;
    }
    if (stride == 0) {
        stride = 1;
    }
    for (size_t i = 0; i < nbElts; ++i) {
        uint8_t const hat = computePrediction8(src, i, stride, mode);
        dst[i] = (uint8_t)(src[i] - hat);
    }
}

void ZS_linearPredictEncode16(
        uint16_t* dst,
        const uint16_t* src,
        size_t nbElts,
        size_t stride,
        uint8_t mode)
{
    if (nbElts == 0 || src == NULL || dst == NULL) {
        return;
    }
    if (stride == 0) {
        stride = 1;
    }
    for (size_t i = 0; i < nbElts; ++i) {
        uint16_t const hat = computePrediction16(src, i, stride, mode);
        dst[i] = (uint16_t)(src[i] - hat);
    }
}

void ZS_linearPredictEncode32(
        uint32_t* dst,
        const uint32_t* src,
        size_t nbElts,
        size_t stride,
        uint8_t mode)
{
    if (nbElts == 0 || src == NULL || dst == NULL) {
        return;
    }
    if (stride == 0) {
        stride = 1;
    }
    for (size_t i = 0; i < nbElts; ++i) {
        uint32_t const hat = computePrediction32(src, i, stride, mode);
        dst[i] = (uint32_t)(src[i] - hat);
    }
}

void ZS_linearPredictEncode64(
        uint64_t* dst,
        const uint64_t* src,
        size_t nbElts,
        size_t stride,
        uint8_t mode)
{
    if (nbElts == 0 || src == NULL || dst == NULL) {
        return;
    }
    if (stride == 0) {
        stride = 1;
    }
    for (size_t i = 0; i < nbElts; ++i) {
        uint64_t const hat = computePrediction64(src, i, stride, mode);
        dst[i] = (uint64_t)(src[i] - hat);
    }
}

void ZS_linearPredictEncode(
        void* dst,
        const void* src,
        size_t nbElts,
        size_t eltWidth,
        size_t stride,
        uint8_t mode)
{
    switch (eltWidth) {
        case 1:
            ZS_linearPredictEncode8((uint8_t*)dst, (const uint8_t*)src, nbElts, stride, mode);
            break;
        case 2:
            ZS_linearPredictEncode16((uint16_t*)dst, (const uint16_t*)src, nbElts, stride, mode);
            break;
        case 4:
            ZS_linearPredictEncode32((uint32_t*)dst, (const uint32_t*)src, nbElts, stride, mode);
            break;
        case 8:
            ZS_linearPredictEncode64((uint64_t*)dst, (const uint64_t*)src, nbElts, stride, mode);
            break;
        default:
            assert(0 && "Unsupported element width");
            break;
    }
}
