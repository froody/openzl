// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZS_DECODE_TANS4_INTERLEAVED_KERNEL_H
#define ZS_DECODE_TANS4_INTERLEAVED_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Reads the decompressed size from a compressed tans4_interleaved buffer.
 * Returns 0 on error or invalid buffer format.
 */
size_t ZS_tans4GetDecompressedSize(const void* src, size_t srcSize);

/**
 * Decodes compressed src into dst using 4-way interleaved tANS.
 * @return Decompressed size in bytes, or 0 on error.
 */
size_t ZS_tans4Decode(
        void* dst,
        size_t dstCapacity,
        const void* src,
        size_t srcSize,
        size_t expectedDstSize);

#ifdef __cplusplus
}
#endif

#endif // ZS_DECODE_TANS4_INTERLEAVED_KERNEL_H
