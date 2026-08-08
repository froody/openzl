// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/elf_erase/decode_elf_erase_kernel.h"
#include <stdlib.h>
#include "openzl/codecs/common/bitstream/ff_bitstream.h"
#include "openzl/shared/bits.h"

int ZS_elfEraseDecode32(uint32_t* dst, const void* src, size_t srcSize, size_t count)
{
    if (count == 0) {
        return 1;
    }
    if (dst == NULL || src == NULL || srcSize == 0) {
        return 0;
    }

    ZS_BitDStreamFF bitstream = ZS_BitDStreamFF_init((const uint8_t*)src, srcSize);

    uint32_t v_prev = 0;
    uint32_t L_prev = 0;
    uint32_t T_prev = 0;
    uint32_t M_prev = 0;

    for (size_t i = 0; i < count; ++i) {
        ZS_BitDStreamFF_reload(&bitstream);
        size_t const c1 = ZS_BitDStreamFF_read(&bitstream, 1);

        uint32_t x_i = 0;
        if (c1 == 0) {
            x_i = 0;
        } else {
            ZS_BitDStreamFF_reload(&bitstream);
            size_t const c2 = ZS_BitDStreamFF_read(&bitstream, 1);
            if (c2 == 0) {
                if (M_prev == 0) {
                    return 0;
                }
                ZS_BitDStreamFF_reload(&bitstream);
                uint32_t val = 0;
                if (M_prev == 32) {
                    val = (uint32_t)ZS_BitDStreamFF_read(&bitstream, 32);
                } else {
                    val = (uint32_t)ZS_BitDStreamFF_read(&bitstream, M_prev);
                }
                x_i = val << T_prev;
            } else {
                ZS_BitDStreamFF_reload(&bitstream);
                uint32_t const L = (uint32_t)ZS_BitDStreamFF_read(&bitstream, 5);
                uint32_t const M = (uint32_t)ZS_BitDStreamFF_read(&bitstream, 6);

                if (L > 31 || M < 1 || M > 32 || L + M > 32) {
                    return 0;
                }
                uint32_t const T = 32 - L - M;

                ZS_BitDStreamFF_reload(&bitstream);
                uint32_t val = 0;
                if (M == 32) {
                    val = (uint32_t)ZS_BitDStreamFF_read(&bitstream, 32);
                } else {
                    val = (uint32_t)ZS_BitDStreamFF_read(&bitstream, M);
                }
                x_i = val << T;

                L_prev = L;
                T_prev = T;
                M_prev = M;
            }
        }

        uint32_t const v_i = v_prev ^ x_i;
        dst[i] = v_i;
        v_prev = v_i;
    }

    ZL_Report const rep = ZS_BitDStreamFF_finish(&bitstream);
    if (ZL_isError(rep)) {
        return 0;
    }
    return 1;
}

int ZS_elfEraseDecode64(uint64_t* dst, const void* src, size_t srcSize, size_t count)
{
    if (count == 0) {
        return 1;
    }
    if (dst == NULL || src == NULL || srcSize == 0) {
        return 0;
    }

    ZS_BitDStreamFF bitstream = ZS_BitDStreamFF_init((const uint8_t*)src, srcSize);

    uint64_t v_prev = 0;
    uint32_t L_prev = 0;
    uint32_t T_prev = 0;
    uint32_t M_prev = 0;

    for (size_t i = 0; i < count; ++i) {
        ZS_BitDStreamFF_reload(&bitstream);
        size_t const c1 = ZS_BitDStreamFF_read(&bitstream, 1);

        uint64_t x_i = 0;
        if (c1 == 0) {
            x_i = 0;
        } else {
            ZS_BitDStreamFF_reload(&bitstream);
            size_t const c2 = ZS_BitDStreamFF_read(&bitstream, 1);
            if (c2 == 0) {
                if (M_prev == 0) {
                    return 0;
                }
                ZS_BitDStreamFF_reload(&bitstream);
                uint64_t val = 0;
                if (M_prev <= 32) {
                    val = ZS_BitDStreamFF_read(&bitstream, M_prev);
                } else {
                    uint64_t const low = ZS_BitDStreamFF_read(&bitstream, 32);
                    ZS_BitDStreamFF_reload(&bitstream);
                    uint64_t const high = ZS_BitDStreamFF_read(&bitstream, M_prev - 32);
                    val = low | (high << 32);
                }
                x_i = val << T_prev;
            } else {
                ZS_BitDStreamFF_reload(&bitstream);
                uint32_t const L = (uint32_t)ZS_BitDStreamFF_read(&bitstream, 6);
                uint32_t const M = (uint32_t)ZS_BitDStreamFF_read(&bitstream, 7);

                if (L > 63 || M < 1 || M > 64 || L + M > 64) {
                    return 0;
                }
                uint32_t const T = 64 - L - M;

                ZS_BitDStreamFF_reload(&bitstream);
                uint64_t val = 0;
                if (M <= 32) {
                    val = ZS_BitDStreamFF_read(&bitstream, M);
                } else {
                    uint64_t const low = ZS_BitDStreamFF_read(&bitstream, 32);
                    ZS_BitDStreamFF_reload(&bitstream);
                    uint64_t const high = ZS_BitDStreamFF_read(&bitstream, M - 32);
                    val = low | (high << 32);
                }
                x_i = val << T;

                L_prev = L;
                T_prev = T;
                M_prev = M;
            }
        }

        uint64_t const v_i = v_prev ^ x_i;
        dst[i] = v_i;
        v_prev = v_i;
    }

    ZL_Report const rep = ZS_BitDStreamFF_finish(&bitstream);
    if (ZL_isError(rep)) {
        return 0;
    }
    return 1;
}

int ZS_elfEraseDecode(void* dst, const void* src, size_t srcSize, size_t count, size_t eltWidth)
{
    if (eltWidth == 4) {
        return ZS_elfEraseDecode32((uint32_t*)dst, src, srcSize, count);
    } else if (eltWidth == 8) {
        return ZS_elfEraseDecode64((uint64_t*)dst, src, srcSize, count);
    } else if (eltWidth == 2) {
        if (count == 0) return 1;
        uint32_t* tmp = (uint32_t*)malloc(count * sizeof(uint32_t));
        if (!tmp) return 0;
        int ok = ZS_elfEraseDecode32(tmp, src, srcSize, count);
        if (ok) {
            uint16_t* dst16 = (uint16_t*)dst;
            for (size_t i = 0; i < count; ++i) dst16[i] = (uint16_t)tmp[i];
        }
        free(tmp);
        return ok;
    } else if (eltWidth == 1) {
        if (count == 0) return 1;
        uint32_t* tmp = (uint32_t*)malloc(count * sizeof(uint32_t));
        if (!tmp) return 0;
        int ok = ZS_elfEraseDecode32(tmp, src, srcSize, count);
        if (ok) {
            uint8_t* dst8 = (uint8_t*)dst;
            for (size_t i = 0; i < count; ++i) dst8[i] = (uint8_t)tmp[i];
        }
        free(tmp);
        return ok;
    }
    return 0;
}
