// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "decode_zigzag_kernel.h"

#include <assert.h>
#include <stdbool.h>
#if defined(__AVX2__)
#    include <immintrin.h>
#endif

void ZL_zigzagDecode64(int64_t* dst, const uint64_t* src, size_t nbElts)
{
    for (size_t i = 0; i < nbElts; i++) {
        uint64_t const z    = src[i];
        uint64_t const mask = 0ULL - (z & 0x1);
        dst[i]              = (int64_t)((z >> 1) ^ mask);
    }
}

void ZL_zigzagDecode32(int32_t* dst, const uint32_t* src, size_t nbElts)
{
    size_t i = 0;
#if defined(__AVX2__)
    for (; i + 8 <= nbElts; i += 8) {
        __m256i v       = _mm256_loadu_si256((__m256i const*)(src + i));
        __m256i v_right = _mm256_srli_epi32(v, 1);
        __m256i v_lsb   = _mm256_and_si256(v, _mm256_set1_epi32(1));
        __m256i v_mask  = _mm256_sub_epi32(_mm256_setzero_si256(), v_lsb);
        __m256i v_dst   = _mm256_xor_si256(v_right, v_mask);
        _mm256_storeu_si256((__m256i*)(dst + i), v_dst);
    }
#endif
    for (; i < nbElts; i++) {
        uint32_t const z    = src[i];
        uint32_t const mask = 0U - (z & 0x1);
        dst[i]              = (int32_t)((z >> 1) ^ mask);
    }
}

void ZL_zigzagDecode16(int16_t* dst, const uint16_t* src, size_t nbElts)
{
    size_t i = 0;
#if defined(__AVX2__)
    for (; i + 16 <= nbElts; i += 16) {
        __m256i v       = _mm256_loadu_si256((__m256i const*)(src + i));
        __m256i v_right = _mm256_srli_epi16(v, 1);
        __m256i v_lsb   = _mm256_and_si256(v, _mm256_set1_epi16(1));
        __m256i v_mask  = _mm256_sub_epi16(_mm256_setzero_si256(), v_lsb);
        __m256i v_dst   = _mm256_xor_si256(v_right, v_mask);
        _mm256_storeu_si256((__m256i*)(dst + i), v_dst);
    }
#endif
    for (; i < nbElts; i++) {
        uint16_t const z    = src[i];
        uint16_t const mask = (uint16_t)0 - (z & 0x1);
        dst[i]              = (int16_t)((z >> 1) ^ mask);
    }
}

void ZL_zigzagDecode8(int8_t* dst, const uint8_t* src, size_t nbElts)
{
    size_t i = 0;
#if defined(__AVX2__)
    for (; i + 32 <= nbElts; i += 32) {
        __m256i v       = _mm256_loadu_si256((__m256i const*)(src + i));
        __m256i v_right = _mm256_and_si256(_mm256_srli_epi16(v, 1), _mm256_set1_epi8(0x7F));
        __m256i v_lsb   = _mm256_and_si256(v, _mm256_set1_epi8(1));
        __m256i v_mask  = _mm256_sub_epi8(_mm256_setzero_si256(), v_lsb);
        __m256i v_dst   = _mm256_xor_si256(v_right, v_mask);
        _mm256_storeu_si256((__m256i*)(dst + i), v_dst);
    }
#endif
    for (; i < nbElts; i++) {
        uint8_t const z    = src[i];
        uint8_t const mask = (uint8_t)0 - (z & 0x1);
        dst[i]             = (int8_t)((z >> 1) ^ mask);
    }
}

void ZL_zigzagDecode(void* dst, const void* src, size_t nbElts, size_t eltWidth)
{
    switch (eltWidth) {
        case 1:
            ZL_zigzagDecode8((int8_t*)dst, (uint8_t const*)src, nbElts);
            break;
        case 2:
            ZL_zigzagDecode16((int16_t*)dst, (uint16_t const*)src, nbElts);
            break;
        case 4:
            ZL_zigzagDecode32((int32_t*)dst, (uint32_t const*)src, nbElts);
            break;
        case 8:
            ZL_zigzagDecode64((int64_t*)dst, (uint64_t const*)src, nbElts);
            break;
        default:
            assert(false);
            break;
    }
}
