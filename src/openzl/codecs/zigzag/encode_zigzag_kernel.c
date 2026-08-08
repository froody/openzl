// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "encode_zigzag_kernel.h"

#include <assert.h>
#include <stdbool.h>
#if defined(__AVX2__)
#    include <immintrin.h>
#endif

void ZL_zigzagEncode64(uint64_t* dst, const int64_t* src, size_t nbElts)
{
    for (size_t i = 0; i < nbElts; i++) {
        int64_t const n = src[i];
        dst[i]          = ((uint64_t)n << 1) ^ (uint64_t)(n >> 63);
    }
}

void ZL_zigzagEncode32(uint32_t* dst, const int32_t* src, size_t nbElts)
{
    size_t i = 0;
#if defined(__AVX2__)
    for (; i + 8 <= nbElts; i += 8) {
        __m256i v        = _mm256_loadu_si256((__m256i const*)(src + i));
        __m256i v_left   = _mm256_slli_epi32(v, 1);
        __m256i v_sign   = _mm256_srai_epi32(v, 31);
        __m256i v_zigzag = _mm256_xor_si256(v_left, v_sign);
        _mm256_storeu_si256((__m256i*)(dst + i), v_zigzag);
    }
#endif
    for (; i < nbElts; i++) {
        int32_t const n = src[i];
        dst[i]          = ((uint32_t)n << 1) ^ (uint32_t)(n >> 31);
    }
}

void ZL_zigzagEncode16(uint16_t* dst, const int16_t* src, size_t nbElts)
{
    size_t i = 0;
#if defined(__AVX2__)
    for (; i + 16 <= nbElts; i += 16) {
        __m256i v        = _mm256_loadu_si256((__m256i const*)(src + i));
        __m256i v_left   = _mm256_slli_epi16(v, 1);
        __m256i v_sign   = _mm256_srai_epi16(v, 15);
        __m256i v_zigzag = _mm256_xor_si256(v_left, v_sign);
        _mm256_storeu_si256((__m256i*)(dst + i), v_zigzag);
    }
#endif
    for (; i < nbElts; i++) {
        int16_t const n = src[i];
        dst[i]          = (uint16_t)(((uint16_t)n << 1) ^ (uint16_t)(n >> 15));
    }
}

void ZL_zigzagEncode8(uint8_t* dst, const int8_t* src, size_t nbElts)
{
    size_t i = 0;
#if defined(__AVX2__)
    for (; i + 32 <= nbElts; i += 32) {
        __m256i v        = _mm256_loadu_si256((__m256i const*)(src + i));
        __m256i v_left   = _mm256_add_epi8(v, v);
        __m256i v_sign   = _mm256_cmpgt_epi8(_mm256_setzero_si256(), v);
        __m256i v_zigzag = _mm256_xor_si256(v_left, v_sign);
        _mm256_storeu_si256((__m256i*)(dst + i), v_zigzag);
    }
#endif
    for (; i < nbElts; i++) {
        int32_t const n = src[i];
        dst[i]          = (uint8_t)(((uint8_t)n << 1) ^ (uint8_t)(n >> 7));
    }
}

void ZL_zigzagEncode(void* dst, const void* src, size_t nbElts, size_t eltWidth)
{
    switch (eltWidth) {
        case 1:
            ZL_zigzagEncode8((uint8_t*)dst, (int8_t const*)src, nbElts);
            break;
        case 2:
            ZL_zigzagEncode16((uint16_t*)dst, (int16_t const*)src, nbElts);
            break;
        case 4:
            ZL_zigzagEncode32((uint32_t*)dst, (int32_t const*)src, nbElts);
            break;
        case 8:
            ZL_zigzagEncode64((uint64_t*)dst, (int64_t const*)src, nbElts);
            break;
        default:
            assert(false);
            break;
    }
}
