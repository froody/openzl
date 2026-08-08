// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/elf_erase/encode_elf_erase_kernel.h"
#include <stdlib.h>
#include "openzl/codecs/common/bitstream/ff_bitstream.h"
#include "openzl/shared/bits.h"

size_t ZS_elfEraseEncode32(void* dst, size_t dstCapacity, const uint32_t* src, size_t count)
{
    if (count == 0) {
        return 0;
    }
    if (dst == NULL || dstCapacity < 4 || src == NULL) {
        return 0;
    }

    ZS_BitCStreamFF bitstream = ZS_BitCStreamFF_init((uint8_t*)dst, dstCapacity);

    uint32_t v_prev = 0;
    uint32_t L_prev = 0;
    uint32_t T_prev = 0;
    uint32_t M_prev = 0;

    for (size_t i = 0; i < count; ++i) {
        uint32_t const v_i = src[i];
        uint32_t const x_i = v_i ^ v_prev;

        if (x_i == 0) {
            ZS_BitCStreamFF_write(&bitstream, 0, 1);
            ZS_BitCStreamFF_flush(&bitstream);
        } else {
            ZS_BitCStreamFF_write(&bitstream, 1, 1);

            uint32_t const L = (uint32_t)ZL_clz32(x_i);
            uint32_t const T = (uint32_t)ZL_ctz32(x_i);
            uint32_t const M = 32 - L - T;

            if (M_prev > 0 && L >= L_prev && T >= T_prev) {
                ZS_BitCStreamFF_write(&bitstream, 0, 1);
                uint32_t const val = (x_i >> T_prev);
                if (M_prev == 32) {
                    ZS_BitCStreamFF_write(&bitstream, val, 32);
                } else {
                    ZS_BitCStreamFF_write(&bitstream, val, M_prev);
                }
                ZS_BitCStreamFF_flush(&bitstream);
            } else {
                ZS_BitCStreamFF_write(&bitstream, 1, 1);
                ZS_BitCStreamFF_write(&bitstream, L, 5);
                ZS_BitCStreamFF_write(&bitstream, M, 6);
                uint32_t const val = (x_i >> T);
                if (M == 32) {
                    ZS_BitCStreamFF_write(&bitstream, val, 32);
                } else {
                    ZS_BitCStreamFF_write(&bitstream, val, M);
                }
                ZS_BitCStreamFF_flush(&bitstream);

                L_prev = L;
                T_prev = T;
                M_prev = M;
            }
        }
        v_prev = v_i;
    }

    ZL_Report const rep = ZS_BitCStreamFF_finish(&bitstream);
    if (ZL_isError(rep)) {
        return 0;
    }
    return ZL_RES_value(rep);
}

size_t ZS_elfEraseEncode64(void* dst, size_t dstCapacity, const uint64_t* src, size_t count)
{
    if (count == 0) {
        return 0;
    }
    if (dst == NULL || dstCapacity < 8 || src == NULL) {
        return 0;
    }

    ZS_BitCStreamFF bitstream = ZS_BitCStreamFF_init((uint8_t*)dst, dstCapacity);

    uint64_t v_prev = 0;
    uint32_t L_prev = 0;
    uint32_t T_prev = 0;
    uint32_t M_prev = 0;

    for (size_t i = 0; i < count; ++i) {
        uint64_t const v_i = src[i];
        uint64_t const x_i = v_i ^ v_prev;

        if (x_i == 0) {
            ZS_BitCStreamFF_write(&bitstream, 0, 1);
            ZS_BitCStreamFF_flush(&bitstream);
        } else {
            ZS_BitCStreamFF_write(&bitstream, 1, 1);

            uint32_t const L = (uint32_t)ZL_clz64(x_i);
            uint32_t const T = (uint32_t)ZL_ctz64(x_i);
            uint32_t const M = 64 - L - T;

            if (M_prev > 0 && L >= L_prev && T >= T_prev) {
                ZS_BitCStreamFF_write(&bitstream, 0, 1);
                uint64_t const val = (x_i >> T_prev);
                if (M_prev <= 32) {
                    ZS_BitCStreamFF_write(&bitstream, (size_t)val, M_prev);
                    ZS_BitCStreamFF_flush(&bitstream);
                } else {
                    ZS_BitCStreamFF_write(&bitstream, (size_t)(val & 0xFFFFFFFFULL), 32);
                    ZS_BitCStreamFF_flush(&bitstream);
                    ZS_BitCStreamFF_write(&bitstream, (size_t)(val >> 32), M_prev - 32);
                    ZS_BitCStreamFF_flush(&bitstream);
                }
            } else {
                ZS_BitCStreamFF_write(&bitstream, 1, 1);
                ZS_BitCStreamFF_write(&bitstream, L, 6);
                ZS_BitCStreamFF_write(&bitstream, M, 7);
                uint64_t const val = (x_i >> T);
                if (M <= 32) {
                    ZS_BitCStreamFF_write(&bitstream, (size_t)val, M);
                    ZS_BitCStreamFF_flush(&bitstream);
                } else {
                    ZS_BitCStreamFF_write(&bitstream, (size_t)(val & 0xFFFFFFFFULL), 32);
                    ZS_BitCStreamFF_flush(&bitstream);
                    ZS_BitCStreamFF_write(&bitstream, (size_t)(val >> 32), M - 32);
                    ZS_BitCStreamFF_flush(&bitstream);
                }

                L_prev = L;
                T_prev = T;
                M_prev = M;
            }
        }
        v_prev = v_i;
    }

    ZL_Report const rep = ZS_BitCStreamFF_finish(&bitstream);
    if (ZL_isError(rep)) {
        return 0;
    }
    return ZL_RES_value(rep);
}

size_t ZS_elfEraseEncode(void* dst, size_t dstCapacity, const void* src, size_t count, size_t eltWidth)
{
    if (eltWidth == 4) {
        return ZS_elfEraseEncode32(dst, dstCapacity, (const uint32_t*)src, count);
    } else if (eltWidth == 8) {
        return ZS_elfEraseEncode64(dst, dstCapacity, (const uint64_t*)src, count);
    } else if (eltWidth == 2) {
        if (count == 0) return 0;
        uint32_t* tmp = (uint32_t*)malloc(count * sizeof(uint32_t));
        if (!tmp) return 0;
        const uint16_t* src16 = (const uint16_t*)src;
        for (size_t i = 0; i < count; ++i) tmp[i] = src16[i];
        size_t res = ZS_elfEraseEncode32(dst, dstCapacity, tmp, count);
        free(tmp);
        return res;
    } else if (eltWidth == 1) {
        if (count == 0) return 0;
        uint32_t* tmp = (uint32_t*)malloc(count * sizeof(uint32_t));
        if (!tmp) return 0;
        const uint8_t* src8 = (const uint8_t*)src;
        for (size_t i = 0; i < count; ++i) tmp[i] = src8[i];
        size_t res = ZS_elfEraseEncode32(dst, dstCapacity, tmp, count);
        free(tmp);
        return res;
    }
    return 0;
}
