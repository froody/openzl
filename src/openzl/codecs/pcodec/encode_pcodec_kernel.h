// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZS_ENCODE_PCODEC_KERNEL_H
#define ZS_ENCODE_PCODEC_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#include "openzl/zl_errors.h"

void ZS_pcodecEncode(
        void* latentsOut,
        void* residualsOut,
        const void* src,
        size_t numElts,
        size_t eltWidth,
        uint64_t baseOffset,
        uint64_t multiplier);

#endif // ZS_ENCODE_PCODEC_KERNEL_H
