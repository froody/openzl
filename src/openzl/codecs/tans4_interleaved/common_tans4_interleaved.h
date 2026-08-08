// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZS_COMMON_TANS4_INTERLEAVED_H
#define ZS_COMMON_TANS4_INTERLEAVED_H

#include <stddef.h>
#include <stdint.h>

enum {
    TANS4_MODE_EMPTY        = 0,
    TANS4_MODE_CONSTANT     = 1,
    TANS4_MODE_UNCOMPRESSED = 2,
    TANS4_MODE_COMPRESSED   = 3,
};

static inline uint16_t readLE16(const uint8_t* p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline uint32_t readLE32(const uint8_t* p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16)
            | ((uint32_t)p[3] << 24);
}

static inline void writeLE16(uint8_t* p, uint16_t val)
{
    p[0] = (uint8_t)(val & 0xFF);
    p[1] = (uint8_t)((val >> 8) & 0xFF);
}

static inline void writeLE32(uint8_t* p, uint32_t val)
{
    p[0] = (uint8_t)(val & 0xFF);
    p[1] = (uint8_t)((val >> 8) & 0xFF);
    p[2] = (uint8_t)((val >> 16) & 0xFF);
    p[3] = (uint8_t)((val >> 24) & 0xFF);
}

static inline int floorLog2(uint32_t x)
{
#if defined(__GNUC__) || defined(__clang__)
    return 31 - __builtin_clz(x);
#else
    int log = 0;
    while (x >>= 1) {
        log++;
    }
    return log;
#endif
}

#endif // ZS_COMMON_TANS4_INTERLEAVED_H
