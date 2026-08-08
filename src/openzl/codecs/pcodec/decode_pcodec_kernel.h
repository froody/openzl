// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZS_DECODE_PCODEC_KERNEL_H
#define ZS_DECODE_PCODEC_KERNEL_H

#include <stddef.h>
#include <stdint.h>

void ZS_pcodecDecode(
        void* dst,
        const void* latents,
        const void* residuals,
        size_t numElts,
        size_t eltWidth,
        uint64_t baseOffset,
        uint64_t multiplier);

#endif // ZS_DECODE_PCODEC_KERNEL_H
