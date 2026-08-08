// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_IWT_DECODE_IWT_KERNEL_H
#define ZSTRONG_TRANSFORMS_IWT_DECODE_IWT_KERNEL_H

#include <stddef.h> // size_t
#include <stdint.h> // uintX_t

#if defined(__cplusplus)
extern "C" {
#endif

void ZS_iwtDecode8(
        uint8_t* dst,
        uint8_t const* srcApprox,
        uint8_t const* srcDetail,
        size_t nbElts);

void ZS_iwtDecode16(
        uint16_t* dst,
        uint16_t const* srcApprox,
        uint16_t const* srcDetail,
        size_t nbElts);

void ZS_iwtDecode32(
        uint32_t* dst,
        uint32_t const* srcApprox,
        uint32_t const* srcDetail,
        size_t nbElts);

void ZS_iwtDecode64(
        uint64_t* dst,
        uint64_t const* srcApprox,
        uint64_t const* srcDetail,
        size_t nbElts);

void ZS_iwtDecode(
        void* dst,
        void const* srcApprox,
        void const* srcDetail,
        size_t nbElts,
        size_t eltWidth);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // ZSTRONG_TRANSFORMS_IWT_DECODE_IWT_KERNEL_H
