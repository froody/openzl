// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_BITPLANE_SPLIT_ENCODE_BITPLANE_SPLIT_KERNEL_H
#define OPENZL_CODECS_BITPLANE_SPLIT_ENCODE_BITPLANE_SPLIT_KERNEL_H

#include <stddef.h> // size_t
#include <stdint.h> // uintX_t

#if defined(__cplusplus)
extern "C" {
#endif

size_t ZS_bitplaneSplit_getMsbWidth(size_t eltWidth, uint8_t msbBits);
size_t ZS_bitplaneSplit_getLsbWidth(size_t eltWidth, uint8_t msbBits);

void ZS_bitplaneSplitEncode8(
        void* dstMsb,
        void* dstLsb,
        uint8_t const* src,
        size_t nbElts,
        uint8_t msbBits);

void ZS_bitplaneSplitEncode16(
        void* dstMsb,
        void* dstLsb,
        uint16_t const* src,
        size_t nbElts,
        uint8_t msbBits);

void ZS_bitplaneSplitEncode32(
        void* dstMsb,
        void* dstLsb,
        uint32_t const* src,
        size_t nbElts,
        uint8_t msbBits);

void ZS_bitplaneSplitEncode64(
        void* dstMsb,
        void* dstLsb,
        uint64_t const* src,
        size_t nbElts,
        uint8_t msbBits);

void ZS_bitplaneSplitEncode(
        void* dstMsb,
        void* dstLsb,
        void const* src,
        size_t nbElts,
        size_t eltWidth,
        uint8_t msbBits);

#if defined(__cplusplus)
}
#endif

#endif // OPENZL_CODECS_BITPLANE_SPLIT_ENCODE_BITPLANE_SPLIT_KERNEL_H
