// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_CAT_PACK_DECODE_CAT_PACK_KERNEL_H
#define ZSTRONG_TRANSFORMS_CAT_PACK_DECODE_CAT_PACK_KERNEL_H

#include "openzl/zl_data.h"
#include "openzl/shared/portability.h"
#include <stddef.h>
#include <stdint.h>

ZL_BEGIN_C_DECLS

// Decodes src of length compressed_len into dest. Returns 1 on success, 0 on failure.
int catPackDecode(void* dest, const void* src, size_t length, size_t compressed_len);

ZL_END_C_DECLS

#endif
