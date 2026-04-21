// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_CAT_PACK_ENCODE_CAT_PACK_KERNEL_H
#define ZSTRONG_TRANSFORMS_CAT_PACK_ENCODE_CAT_PACK_KERNEL_H

#include "openzl/zl_data.h"
#include "openzl/shared/portability.h"
#include <stddef.h>
#include <stdint.h>

ZL_BEGIN_C_DECLS

// Encodes src into dst. Returns the number of bytes written, or 0 if it fails (too many classes).
size_t catPackEncode(void* dst, const void* src, size_t length);

ZL_END_C_DECLS

#endif
