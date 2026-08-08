// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_IWT_ENCODE_IWT_KERNEL_H
#define ZSTRONG_TRANSFORMS_IWT_ENCODE_IWT_KERNEL_H

#include <stddef.h> // size_t
#include <stdint.h> // uintX_t

#if defined(__cplusplus)
extern "C" {
#endif

void ZS_iwtEncode8(
        uint8_t* dstApprox,
        uint8_t* dstDetail,
        uint8_t const* src,
        size_t nbElts);

void ZS_iwtEncode16(
        uint16_t* dstApprox,
        uint16_t* dstDetail,
        uint16_t const* src,
        size_t nbElts);

void ZS_iwtEncode32(
        uint32_t* dstApprox,
        uint32_t* dstDetail,
        uint32_t const* src,
        size_t nbElts);

void ZS_iwtEncode64(
        uint64_t* dstApprox,
        uint64_t* dstDetail,
        uint64_t const* src,
        size_t nbElts);

void ZS_iwtEncode(
        void* dstApprox,
        void* dstDetail,
        void const* src,
        size_t nbElts,
        size_t eltWidth);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // ZSTRONG_TRANSFORMS_IWT_ENCODE_IWT_KERNEL_H
