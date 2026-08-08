// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/morton_3d/encode_morton_3d_kernel.h"
#include "openzl/common/assertion.h"

void ZS_morton3dEncode8(
        uint64_t* dst,
        const uint8_t* srcX,
        const uint8_t* srcY,
        const uint8_t* srcZ,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(srcX);
    ZL_ASSERT_NN(srcY);
    ZL_ASSERT_NN(srcZ);

    for (size_t i = 0; i < count; ++i) {
        dst[i] = ZS_morton3d_encode_single(srcX[i], srcY[i], srcZ[i]);
    }
}

void ZS_morton3dEncode16(
        uint64_t* dst,
        const uint16_t* srcX,
        const uint16_t* srcY,
        const uint16_t* srcZ,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(srcX);
    ZL_ASSERT_NN(srcY);
    ZL_ASSERT_NN(srcZ);

    for (size_t i = 0; i < count; ++i) {
        dst[i] = ZS_morton3d_encode_single(srcX[i], srcY[i], srcZ[i]);
    }
}

void ZS_morton3dEncode32(
        uint64_t* dst,
        const uint32_t* srcX,
        const uint32_t* srcY,
        const uint32_t* srcZ,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(srcX);
    ZL_ASSERT_NN(srcY);
    ZL_ASSERT_NN(srcZ);

    for (size_t i = 0; i < count; ++i) {
        dst[i] = ZS_morton3d_encode_single(srcX[i], srcY[i], srcZ[i]);
    }
}

void ZS_morton3dEncode64(
        uint64_t* dst,
        const uint64_t* srcX,
        const uint64_t* srcY,
        const uint64_t* srcZ,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(srcX);
    ZL_ASSERT_NN(srcY);
    ZL_ASSERT_NN(srcZ);

    for (size_t i = 0; i < count; ++i) {
        dst[i] = ZS_morton3d_encode_single(srcX[i], srcY[i], srcZ[i]);
    }
}

void ZS_morton3dEncodeInterleaved8(
        uint64_t* dst,
        const uint8_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        dst[i] = ZS_morton3d_encode_single(src[3 * i], src[3 * i + 1], src[3 * i + 2]);
    }
}

void ZS_morton3dEncodeInterleaved16(
        uint64_t* dst,
        const uint16_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        dst[i] = ZS_morton3d_encode_single(src[3 * i], src[3 * i + 1], src[3 * i + 2]);
    }
}

void ZS_morton3dEncodeInterleaved32(
        uint64_t* dst,
        const uint32_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        dst[i] = ZS_morton3d_encode_single(src[3 * i], src[3 * i + 1], src[3 * i + 2]);
    }
}

void ZS_morton3dEncodeInterleaved64(
        uint64_t* dst,
        const uint64_t* src,
        size_t count)
{
    if (count == 0) return;
    ZL_ASSERT_NN(dst);
    ZL_ASSERT_NN(src);

    for (size_t i = 0; i < count; ++i) {
        dst[i] = ZS_morton3d_encode_single(src[3 * i], src[3 * i + 1], src[3 * i + 2]);
    }
}

void ZS_morton3dEncode(
        uint64_t* dst,
        const void* srcX,
        const void* srcY,
        const void* srcZ,
        size_t count,
        size_t coordWidth)
{
    if (count == 0) return;

    switch (coordWidth) {
        case 1:
            ZS_morton3dEncode8(
                    dst,
                    (const uint8_t*)srcX,
                    (const uint8_t*)srcY,
                    (const uint8_t*)srcZ,
                    count);
            break;
        case 2:
            ZS_morton3dEncode16(
                    dst,
                    (const uint16_t*)srcX,
                    (const uint16_t*)srcY,
                    (const uint16_t*)srcZ,
                    count);
            break;
        case 4:
            ZS_morton3dEncode32(
                    dst,
                    (const uint32_t*)srcX,
                    (const uint32_t*)srcY,
                    (const uint32_t*)srcZ,
                    count);
            break;
        case 8:
            ZS_morton3dEncode64(
                    dst,
                    (const uint64_t*)srcX,
                    (const uint64_t*)srcY,
                    (const uint64_t*)srcZ,
                    count);
            break;
        default:
            ZL_ASSERT_FAIL("Unsupported coordinate width");
    }
}

void ZS_morton3dEncodeInterleaved(
        uint64_t* dst,
        const void* src,
        size_t count,
        size_t coordWidth)
{
    if (count == 0) return;

    switch (coordWidth) {
        case 1:
            ZS_morton3dEncodeInterleaved8(dst, (const uint8_t*)src, count);
            break;
        case 2:
            ZS_morton3dEncodeInterleaved16(dst, (const uint16_t*)src, count);
            break;
        case 4:
            ZS_morton3dEncodeInterleaved32(dst, (const uint32_t*)src, count);
            break;
        case 8:
            ZS_morton3dEncodeInterleaved64(dst, (const uint64_t*)src, count);
            break;
        default:
            ZL_ASSERT_FAIL("Unsupported coordinate width");
    }
}
