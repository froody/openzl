// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_MORTON_3D_ENCODE_MORTON_3D_KERNEL_H
#define OPENZL_CODECS_MORTON_3D_ENCODE_MORTON_3D_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/// Expands up to 21 bits of @p v into bits at positions 0, 3, 6, 9, ... 60.
static inline uint64_t ZS_morton3d_expand_bits(uint64_t v)
{
    v &= 0x1fffffULL;
    v = (v | (v << 32)) & 0x1f00000000ffffULL;
    v = (v | (v << 16)) & 0x1f0000ff0000ffULL;
    v = (v | (v << 8))  & 0x100f00f00f00f00fULL;
    v = (v | (v << 4))  & 0x10c30c30c30c30c3ULL;
    v = (v | (v << 2))  & 0x1249249249249249ULL;
    return v;
}

/// Interleaves bits of (x, y, z) into a 64-bit Morton code.
static inline uint64_t ZS_morton3d_encode_single(uint64_t x, uint64_t y, uint64_t z)
{
    return ZS_morton3d_expand_bits(x) |
           (ZS_morton3d_expand_bits(y) << 1) |
           (ZS_morton3d_expand_bits(z) << 2);
}

void ZS_morton3dEncode8(
        uint64_t* dst,
        const uint8_t* srcX,
        const uint8_t* srcY,
        const uint8_t* srcZ,
        size_t count);

void ZS_morton3dEncode16(
        uint64_t* dst,
        const uint16_t* srcX,
        const uint16_t* srcY,
        const uint16_t* srcZ,
        size_t count);

void ZS_morton3dEncode32(
        uint64_t* dst,
        const uint32_t* srcX,
        const uint32_t* srcY,
        const uint32_t* srcZ,
        size_t count);

void ZS_morton3dEncode64(
        uint64_t* dst,
        const uint64_t* srcX,
        const uint64_t* srcY,
        const uint64_t* srcZ,
        size_t count);

void ZS_morton3dEncodeInterleaved8(
        uint64_t* dst,
        const uint8_t* src,
        size_t count);

void ZS_morton3dEncodeInterleaved16(
        uint64_t* dst,
        const uint16_t* src,
        size_t count);

void ZS_morton3dEncodeInterleaved32(
        uint64_t* dst,
        const uint32_t* src,
        size_t count);

void ZS_morton3dEncodeInterleaved64(
        uint64_t* dst,
        const uint64_t* src,
        size_t count);

void ZS_morton3dEncode(
        uint64_t* dst,
        const void* srcX,
        const void* srcY,
        const void* srcZ,
        size_t count,
        size_t coordWidth);

void ZS_morton3dEncodeInterleaved(
        uint64_t* dst,
        const void* src,
        size_t count,
        size_t coordWidth);

#if defined(__cplusplus)
}
#endif

#endif // OPENZL_CODECS_MORTON_3D_ENCODE_MORTON_3D_KERNEL_H
