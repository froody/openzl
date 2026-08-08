// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_ELF_ERASE_DECODE_KERNEL_H
#define ZSTRONG_TRANSFORMS_ELF_ERASE_DECODE_KERNEL_H

#include <stddef.h>
#include <stdint.h>
#include "openzl/shared/portability.h"

ZL_BEGIN_C_DECLS

/**
 * Decodes 32-bit floats/integers from an elf_erase bitstream.
 *
 * @param dst Output buffer for 32-bit elements
 * @param src Input compressed bitstream
 * @param srcSize Input bitstream size in bytes
 * @param count Number of 32-bit elements to decode
 * @return 1 on success, 0 on error / corruption
 */
int ZS_elfEraseDecode32(uint32_t* dst, const void* src, size_t srcSize, size_t count);

/**
 * Decodes 64-bit floats/integers from an elf_erase bitstream.
 *
 * @param dst Output buffer for 64-bit elements
 * @param src Input compressed bitstream
 * @param srcSize Input bitstream size in bytes
 * @param count Number of 64-bit elements to decode
 * @return 1 on success, 0 on error / corruption
 */
int ZS_elfEraseDecode64(uint64_t* dst, const void* src, size_t srcSize, size_t count);

/**
 * Generic entry point dispatching by element width (1, 2, 4, or 8 bytes).
 */
int ZS_elfEraseDecode(void* dst, const void* src, size_t srcSize, size_t count, size_t eltWidth);

ZL_END_C_DECLS

#endif // ZSTRONG_TRANSFORMS_ELF_ERASE_DECODE_KERNEL_H
