// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_BITPLANE_SPLIT_DECODE_BITPLANE_SPLIT_KERNEL_H
#define OPENZL_CODECS_BITPLANE_SPLIT_DECODE_BITPLANE_SPLIT_KERNEL_H

#include <stddef.h> // size_t
#include <stdint.h> // uintX_t

#if defined(__cplusplus)
extern "C" {
#endif

void ZS_bitplaneSplitDecode8(
        uint8_t* dst,
        void const* srcMsb,
        void const* srcLsb,
        size_t nbElts,
        uint8_t msbBits);

void ZS_bitplaneSplitDecode16(
        uint16_t* dst,
        void const* srcMsb,
        void const* srcLsb,
        size_t nbElts,
        uint8_t msbBits);

void ZS_bitplaneSplitDecode32(
        uint32_t* dst,
        void const* srcMsb,
        void const* srcLsb,
        size_t nbElts,
        uint8_t msbBits);

void ZS_bitplaneSplitDecode64(
        uint64_t* dst,
        void const* srcMsb,
        void const* srcLsb,
        size_t nbElts,
        uint8_t msbBits);

void ZS_bitplaneSplitDecode(
        void* dst,
        void const* srcMsb,
        void const* srcLsb,
        size_t nbElts,
        size_t eltWidth,
        uint8_t msbBits);

#if defined(__cplusplus)
}
#endif

#endif // OPENZL_CODECS_BITPLANE_SPLIT_DECODE_BITPLANE_SPLIT_KERNEL_H
