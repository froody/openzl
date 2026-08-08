// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_MORTON_3D_DECODE_MORTON_3D_KERNEL_H
#define OPENZL_CODECS_MORTON_3D_DECODE_MORTON_3D_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/// Compacts bits from positions 0, 3, 6, 9, ... 60 down to contiguous LSBs 0..20.
static inline uint64_t ZS_morton3d_compact_bits(uint64_t v)
{
    v &= 0x1249249249249249ULL;
    v = (v | (v >> 2))  & 0x10c30c30c30c30c3ULL;
    v = (v | (v >> 4))  & 0x100f00f00f00f00fULL;
    v = (v | (v >> 8))  & 0x1f0000ff0000ffULL;
    v = (v | (v >> 16)) & 0x1f00000000ffffULL;
    v = (v | (v >> 32)) & 0x1fffffULL;
    return v;
}

/// Decodes a 64-bit Morton code into 21-bit coordinates (x, y, z).
static inline void ZS_morton3d_decode_single(
        uint64_t code,
        uint64_t* x,
        uint64_t* y,
        uint64_t* z)
{
    *x = ZS_morton3d_compact_bits(code);
    *y = ZS_morton3d_compact_bits(code >> 1);
    *z = ZS_morton3d_compact_bits(code >> 2);
}

void ZS_morton3dDecode8(
        uint8_t* dstX,
        uint8_t* dstY,
        uint8_t* dstZ,
        const uint64_t* src,
        size_t count);

void ZS_morton3dDecode16(
        uint16_t* dstX,
        uint16_t* dstY,
        uint16_t* dstZ,
        const uint64_t* src,
        size_t count);

void ZS_morton3dDecode32(
        uint32_t* dstX,
        uint32_t* dstY,
        uint32_t* dstZ,
        const uint64_t* src,
        size_t count);

void ZS_morton3dDecode64(
        uint64_t* dstX,
        uint64_t* dstY,
        uint64_t* dstZ,
        const uint64_t* src,
        size_t count);

void ZS_morton3dDecodeInterleaved8(
        uint8_t* dst,
        const uint64_t* src,
        size_t count);

void ZS_morton3dDecodeInterleaved16(
        uint16_t* dst,
        const uint64_t* src,
        size_t count);

void ZS_morton3dDecodeInterleaved32(
        uint32_t* dst,
        const uint64_t* src,
        size_t count);

void ZS_morton3dDecodeInterleaved64(
        uint64_t* dst,
        const uint64_t* src,
        size_t count);

void ZS_morton3dDecode(
        void* dstX,
        void* dstY,
        void* dstZ,
        const uint64_t* src,
        size_t count,
        size_t coordWidth);

void ZS_morton3dDecodeInterleaved(
        void* dst,
        const uint64_t* src,
        size_t count,
        size_t coordWidth);

#if defined(__cplusplus)
}
#endif

#endif // OPENZL_CODECS_MORTON_3D_DECODE_MORTON_3D_KERNEL_H
