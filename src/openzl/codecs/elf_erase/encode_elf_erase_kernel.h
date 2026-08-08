// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_ELF_ERASE_ENCODE_KERNEL_H
#define ZSTRONG_TRANSFORMS_ELF_ERASE_ENCODE_KERNEL_H

#include <stddef.h>
#include <stdint.h>
#include "openzl/shared/portability.h"

ZL_BEGIN_C_DECLS

/**
 * Encodes 32-bit floats/integers into a compact bitstream by erasing trailing XOR zero bits.
 *
 * @param dst Destination buffer
 * @param dstCapacity Maximum capacity of dst in bytes
 * @param src Source array of 32-bit integers
 * @param count Number of elements in src
 * @return Bytes written to dst, or 0 on error
 */
size_t ZS_elfEraseEncode32(void* dst, size_t dstCapacity, const uint32_t* src, size_t count);

/**
 * Encodes 64-bit floats/integers into a compact bitstream by erasing trailing XOR zero bits.
 *
 * @param dst Destination buffer
 * @param dstCapacity Maximum capacity of dst in bytes
 * @param src Source array of 64-bit integers
 * @param count Number of elements in src
 * @return Bytes written to dst, or 0 on error
 */
size_t ZS_elfEraseEncode64(void* dst, size_t dstCapacity, const uint64_t* src, size_t count);

/**
 * Generic entry point dispatching by element width (1, 2, 4, or 8 bytes).
 */
size_t ZS_elfEraseEncode(void* dst, size_t dstCapacity, const void* src, size_t count, size_t eltWidth);

ZL_END_C_DECLS

#endif // ZSTRONG_TRANSFORMS_ELF_ERASE_ENCODE_KERNEL_H
