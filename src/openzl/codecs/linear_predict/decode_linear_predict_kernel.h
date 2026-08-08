// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_LINEAR_PREDICT_DECODE_KERNEL_H
#define ZSTRONG_TRANSFORMS_LINEAR_PREDICT_DECODE_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

void ZS_linearPredictDecode8(
        uint8_t* dst,
        const uint8_t* src,
        size_t nbElts,
        size_t stride,
        uint8_t mode);

void ZS_linearPredictDecode16(
        uint16_t* dst,
        const uint16_t* src,
        size_t nbElts,
        size_t stride,
        uint8_t mode);

void ZS_linearPredictDecode32(
        uint32_t* dst,
        const uint32_t* src,
        size_t nbElts,
        size_t stride,
        uint8_t mode);

void ZS_linearPredictDecode64(
        uint64_t* dst,
        const uint64_t* src,
        size_t nbElts,
        size_t stride,
        uint8_t mode);

void ZS_linearPredictDecode(
        void* dst,
        const void* src,
        size_t nbElts,
        size_t eltWidth,
        size_t stride,
        uint8_t mode);

#if defined(__cplusplus)
}
#endif

#endif // ZSTRONG_TRANSFORMS_LINEAR_PREDICT_DECODE_KERNEL_H
