// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/morton_3d/decode_morton_3d_kernel.h"
#include "openzl/common/assertion.h"

void ZS_morton3dDecode8(
        uint8_t* dstX,
        uint8_t* dstY,
        uint8_t* dstZ,
        const uint64_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dstX);
    ZL_ASSERT_NN(dstY);
    ZL_ASSERT_NN(dstZ);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        uint64_t x, y, z;
        ZS_morton3d_decode_single(src[i], &x, &y, &z);
        dstX[i] = (uint8_t)x;
        dstY[i] = (uint8_t)y;
        dstZ[i] = (uint8_t)z;
    }
}

void ZS_morton3dDecode16(
        uint16_t* dstX,
        uint16_t* dstY,
        uint16_t* dstZ,
        const uint64_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dstX);
    ZL_ASSERT_NN(dstY);
    ZL_ASSERT_NN(dstZ);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        uint64_t x, y, z;
        ZS_morton3d_decode_single(src[i], &x, &y, &z);
        dstX[i] = (uint16_t)x;
        dstY[i] = (uint16_t)y;
        dstZ[i] = (uint16_t)z;
    }
}

void ZS_morton3dDecode32(
        uint32_t* dstX,
        uint32_t* dstY,
        uint32_t* dstZ,
        const uint64_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dstX);
    ZL_ASSERT_NN(dstY);
    ZL_ASSERT_NN(dstZ);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        uint64_t x, y, z;
        ZS_morton3d_decode_single(src[i], &x, &y, &z);
        dstX[i] = (uint32_t)x;
        dstY[i] = (uint32_t)y;
        dstZ[i] = (uint32_t)z;
    }
}

void ZS_morton3dDecode64(
        uint64_t* dstX,
        uint64_t* dstY,
        uint64_t* dstZ,
        const uint64_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dstX);
    ZL_ASSERT_NN(dstY);
    ZL_ASSERT_NN(dstZ);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        uint64_t x, y, z;
        ZS_morton3d_decode_single(src[i], &x, &y, &z);
        dstX[i] = x;
        dstY[i] = y;
        dstZ[i] = z;
    }
}

void ZS_morton3dDecodeInterleaved8(
        uint8_t* dst,
        const uint64_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        uint64_t x, y, z;
        ZS_morton3d_decode_single(src[i], &x, &y, &z);
        dst[3 * i]     = (uint8_t)x;
        dst[3 * i + 1] = (uint8_t)y;
        dst[3 * i + 2] = (uint8_t)z;
    }
}

void ZS_morton3dDecodeInterleaved16(
        uint16_t* dst,
        const uint64_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        uint64_t x, y, z;
        ZS_morton3d_decode_single(src[i], &x, &y, &z);
        dst[3 * i]     = (uint16_t)x;
        dst[3 * i + 1] = (uint16_t)y;
        dst[3 * i + 2] = (uint16_t)z;
    }
}

void ZS_morton3dDecodeInterleaved32(
        uint32_t* dst,
        const uint64_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        uint64_t x, y, z;
        ZS_morton3d_decode_single(src[i], &x, &y, &z);
        dst[3 * i]     = (uint32_t)x;
        dst[3 * i + 1] = (uint32_t)y;
        dst[3 * i + 2] = (uint32_t)z;
    }
}

void ZS_morton3dDecodeInterleaved64(
        uint64_t* dst,
        const uint64_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        uint64_t x, y, z;
        ZS_morton3d_decode_single(src[i], &x, &y, &z);
        dst[3 * i]     = x;
        dst[3 * i + 1] = y;
        dst[3 * i + 2] = z;
    }
}

void ZS_morton3dDecode(
        void* dstX,
        void* dstY,
        void* dstZ,
        const uint64_t* src,
        size_t count,
        size_t coordWidth)
{
    if (count == 0) return;

    switch (coordWidth) {
        case 1:
            ZS_morton3dDecode8(
                    (uint8_t*)dstX,
                    (uint8_t*)dstY,
                    (uint8_t*)dstZ,
                    src,
                    count);
            break;
        case 2:
            ZS_morton3dDecode16(
                    (uint16_t*)dstX,
                    (uint16_t*)dstY,
                    (uint16_t*)dstZ,
                    src,
                    count);
            break;
        case 4:
            ZS_morton3dDecode32(
                    (uint32_t*)dstX,
                    (uint32_t*)dstY,
                    (uint32_t*)dstZ,
                    src,
                    count);
            break;
        case 8:
            ZS_morton3dDecode64(
                    (uint64_t*)dstX,
                    (uint64_t*)dstY,
                    (uint64_t*)dstZ,
                    src,
                    count);
            break;
        default:
            ZL_ASSERT_FAIL("Unsupported coordinate width");
    }
}

void ZS_morton3dDecodeInterleaved(
        void* dst,
        const uint64_t* src,
        size_t count,
        size_t coordWidth)
{
    if (count == 0) return;

    switch (coordWidth) {
        case 1:
            ZS_morton3dDecodeInterleaved8((uint8_t*)dst, src, count);
            break;
        case 2:
            ZS_morton3dDecodeInterleaved16((uint16_t*)dst, src, count);
            break;
        case 4:
            ZS_morton3dDecodeInterleaved32((uint32_t*)dst, src, count);
            break;
        case 8:
            ZS_morton3dDecodeInterleaved64((uint64_t*)dst, src, count);
            break;
        default:
            ZL_ASSERT_FAIL("Unsupported coordinate width");
    }
}
