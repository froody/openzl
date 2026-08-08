// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/tans4_interleaved/decode_tans4_interleaved_kernel.h"
#include "openzl/codecs/tans4_interleaved/common_tans4_interleaved.h"
#include "openzl/codecs/common/bitstream/bf_bitstream.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t symbol;
    uint8_t nbBits;
    uint16_t newStateBase;
} TANS4_DecSymbol;

size_t ZS_tans4GetDecompressedSize(const void* src, size_t srcSize)
{
    if (src == NULL || srcSize < 6) {
        return 0;
    }
    const uint8_t* in = (const uint8_t*)src;
    return (size_t)readLE32(in);
}

size_t ZS_tans4Decode(
        void* dst,
        size_t dstCapacity,
        const void* src,
        size_t srcSize,
        size_t expectedDstSize)
{
    if (src == NULL || srcSize < 6 || dst == NULL) {
        return 0;
    }

    const uint8_t* in     = (const uint8_t*)src;
    size_t origSize       = (size_t)readLE32(in);
    uint8_t eltWidth      = in[4];
    uint8_t mode          = in[5];

    if (origSize != expectedDstSize || eltWidth != 1 || dstCapacity < origSize) {
        return 0;
    }

    uint8_t* out = (uint8_t*)dst;

    if (mode == TANS4_MODE_EMPTY) {
        return (origSize == 0) ? 0 : 0;
    }

    if (mode == TANS4_MODE_CONSTANT) {
        if (srcSize < 7) return 0;
        memset(out, in[6], origSize);
        return origSize;
    }

    if (mode == TANS4_MODE_UNCOMPRESSED) {
        if (srcSize < 6 + origSize) return 0;
        memcpy(out, in + 6, origSize);
        return origSize;
    }

    if (mode != TANS4_MODE_COMPRESSED) {
        return 0;
    }

    const uint8_t* hPtr = in + 6;
    if (srcSize < 6 + 1 + 2) return 0;

    uint32_t tableLog  = *hPtr++;
    if (tableLog < 8 || tableLog > 11) return 0;

    uint32_t maxSymbol = readLE16(hPtr);
    hPtr += 2;
    if (maxSymbol >= 256) return 0;

    size_t freqHeaderLen = (maxSymbol + 1) * 2;
    if ((size_t)(in + srcSize - hPtr) < freqHeaderLen + 16 + 8) return 0;

    uint16_t normFreqs[256];
    memset(normFreqs, 0, sizeof(normFreqs));
    uint32_t sumFreqs = 0;

    for (uint32_t s = 0; s <= maxSymbol; ++s) {
        normFreqs[s] = readLE16(hPtr);
        hPtr += 2;
        sumFreqs += normFreqs[s];
    }

    uint32_t L = 1U << tableLog;
    if (sumFreqs != L) return 0;

    uint32_t streamSize[4];
    for (int j = 0; j < 4; ++j) {
        streamSize[j] = readLE32(hPtr);
        hPtr += 4;
    }

    uint32_t finalState[4];
    for (int j = 0; j < 4; ++j) {
        finalState[j] = (uint32_t)readLE16(hPtr);
        hPtr += 2;
        if (finalState[j] >= L) return 0;
    }

    size_t payloadTotal = 0;
    for (int j = 0; j < 4; ++j) {
        payloadTotal += streamSize[j];
    }

    if ((size_t)(in + srcSize - hPtr) < payloadTotal) {
        return 0;
    }

    TANS4_DecSymbol decTable[2048];
    uint32_t step = (L >> 1) + (L >> 3) + 3;
    uint32_t pos  = 0;

    for (uint32_t s = 0; s <= maxSymbol; ++s) {
        uint16_t f = normFreqs[s];
        for (uint16_t i = 0; i < f; ++i) {
            uint32_t x             = f + i;
            int nbBits             = (int)tableLog - floorLog2(x);
            uint32_t newStateBase  = (x << nbBits) - L;

            decTable[pos].symbol       = (uint8_t)s;
            decTable[pos].nbBits       = (uint8_t)nbBits;
            decTable[pos].newStateBase = (uint16_t)newStateBase;

            pos = (pos + step) & (L - 1);
        }
    }

    ZS_BitDStreamBF reader[4];
    const uint8_t* pData = hPtr;
    for (int j = 0; j < 4; ++j) {
        reader[j] = ZS_BitDStreamBF_init(pData, streamSize[j]);
        pData += streamSize[j];
    }

    uint32_t x[4];
    for (int j = 0; j < 4; ++j) {
        x[j] = L + finalState[j];
    }

    for (size_t i = 0; i < origSize; i += 4) {
        if (i + 0 < origSize) {
            TANS4_DecSymbol e0 = decTable[x[0] - L];
            out[i + 0]         = e0.symbol;
            ZS_BitDStreamBF_reload(&reader[0]);
            uint32_t bits0     = (uint32_t)ZS_BitDStreamBF_read(&reader[0], e0.nbBits);
            x[0]               = L + e0.newStateBase + bits0;
        }
        if (i + 1 < origSize) {
            TANS4_DecSymbol e1 = decTable[x[1] - L];
            out[i + 1]         = e1.symbol;
            ZS_BitDStreamBF_reload(&reader[1]);
            uint32_t bits1     = (uint32_t)ZS_BitDStreamBF_read(&reader[1], e1.nbBits);
            x[1]               = L + e1.newStateBase + bits1;
        }
        if (i + 2 < origSize) {
            TANS4_DecSymbol e2 = decTable[x[2] - L];
            out[i + 2]         = e2.symbol;
            ZS_BitDStreamBF_reload(&reader[2]);
            uint32_t bits2     = (uint32_t)ZS_BitDStreamBF_read(&reader[2], e2.nbBits);
            x[2]               = L + e2.newStateBase + bits2;
        }
        if (i + 3 < origSize) {
            TANS4_DecSymbol e3 = decTable[x[3] - L];
            out[i + 3]         = e3.symbol;
            ZS_BitDStreamBF_reload(&reader[3]);
            uint32_t bits3     = (uint32_t)ZS_BitDStreamBF_read(&reader[3], e3.nbBits);
            x[3]               = L + e3.newStateBase + bits3;
        }
    }

    return origSize;
}
