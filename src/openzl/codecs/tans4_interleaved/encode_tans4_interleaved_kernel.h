// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZS_ENCODE_TANS4_INTERLEAVED_KERNEL_H
#define ZS_ENCODE_TANS4_INTERLEAVED_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TANS4_TABLELOG_DEFAULT 10
#define TANS4_TABLELOG_MAX 11
#define TANS4_L_DEFAULT (1U << TANS4_TABLELOG_DEFAULT)
#define TANS4_MAX_SYMBOLS 256

/**
 * Returns maximum buffer capacity required to encode srcSize bytes.
 */
size_t ZS_tans4EncodeBound(size_t srcSize);

/**
 * Encodes src into dst using 4-way interleaved tANS.
 * @return Compressed size in bytes, or 0 on error/failure.
 */
size_t ZS_tans4Encode(
        void* dst,
        size_t dstCapacity,
        const void* src,
        size_t srcSize);

#ifdef __cplusplus
}
#endif

#endif // ZS_ENCODE_TANS4_INTERLEAVED_KERNEL_H
