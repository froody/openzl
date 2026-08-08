// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_SPHERICAL_RANGE_ENCODE_KERNEL_H
#define ZSTRONG_TRANSFORMS_SPHERICAL_RANGE_ENCODE_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

void ZS_sphericalRangeEncode8(uint8_t* dst, uint8_t const* src, size_t nbElts);
void ZS_sphericalRangeEncode16(uint16_t* dst, uint16_t const* src, size_t nbElts);
void ZS_sphericalRangeEncode32(uint32_t* dst, uint32_t const* src, size_t nbElts);
void ZS_sphericalRangeEncode64(uint64_t* dst, uint64_t const* src, size_t nbElts);

void ZS_sphericalRangeEncode(
        void* dst,
        void const* src,
        size_t nbElts,
        size_t eltWidth);

#if defined(__cplusplus)
}
#endif

#endif // ZSTRONG_TRANSFORMS_SPHERICAL_RANGE_ENCODE_KERNEL_H
