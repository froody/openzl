// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/tans4_interleaved/encode_tans4_interleaved_kernel.h"
#include "openzl/codecs/tans4_interleaved/common_tans4_interleaved.h"
#include "openzl/codecs/common/bitstream/bf_bitstream.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    uint16_t freq;
    uint16_t k_base;
    uint32_t offset;
} TANS4_EncSymbol;

size_t ZS_tans4EncodeBound(size_t srcSize)
{
    return 1024 + srcSize + (srcSize / 8);
}

size_t ZS_tans4Encode(
        void* dst,
        size_t dstCapacity,
        const void* src,
        size_t srcSize)
{
    if (dst == NULL || dstCapacity < 8) {
        return 0;
    }

    uint8_t* out = (uint8_t*)dst;
    writeLE32(out, (uint32_t)srcSize);
    out[4] = 1; // eltWidth = 1

    if (srcSize == 0) {
        out[5] = TANS4_MODE_EMPTY;
        return 6;
    }

    if (src == NULL) {
        return 0;
    }

    const uint8_t* bytes = (const uint8_t*)src;

    // Check for constant
    int isConstant = 1;
    for (size_t i = 1; i < srcSize; ++i) {
        if (bytes[i] != bytes[0]) {
            isConstant = 0;
            break;
        }
    }
    if (isConstant) {
        if (dstCapacity < 7) return 0;
        out[5] = TANS4_MODE_CONSTANT;
        out[6] = bytes[0];
        return 7;
    }

    // Build histogram
    uint32_t count[TANS4_MAX_SYMBOLS];
    memset(count, 0, sizeof(count));
    for (size_t i = 0; i < srcSize; ++i) {
        count[bytes[i]]++;
    }

    uint32_t maxSymbol = 0;
    for (uint32_t s = 0; s < TANS4_MAX_SYMBOLS; ++s) {
        if (count[s] > 0) {
            maxSymbol = s;
        }
    }

    const uint32_t tableLog = TANS4_TABLELOG_DEFAULT;
    const uint32_t L        = 1U << tableLog;

    // Normalize frequencies
    uint16_t normFreqs[TANS4_MAX_SYMBOLS];
    memset(normFreqs, 0, sizeof(normFreqs));

    uint32_t totalNorm = 0;
    for (uint32_t s = 0; s <= maxSymbol; ++s) {
        if (count[s] > 0) {
            uint32_t f = (uint32_t)(((uint64_t)count[s] * L) / srcSize);
            if (f == 0) f = 1;
            normFreqs[s] = (uint16_t)f;
            totalNorm += f;
        }
    }

    // Adjust frequencies to sum to L exactly
    while (totalNorm < L) {
        uint32_t bestSym = 0;
        uint64_t maxRem  = 0;
        for (uint32_t s = 0; s <= maxSymbol; ++s) {
            if (count[s] > 0) {
                uint64_t rem = ((uint64_t)count[s] * L) % srcSize;
                if (rem >= maxRem) {
                    maxRem  = rem;
                    bestSym = s;
                }
            }
        }
        normFreqs[bestSym]++;
        totalNorm++;
    }

    while (totalNorm > L) {
        uint32_t bestSym = 0;
        uint32_t maxF    = 0;
        for (uint32_t s = 0; s <= maxSymbol; ++s) {
            if (normFreqs[s] > 1 && normFreqs[s] >= maxF) {
                maxF    = normFreqs[s];
                bestSym = s;
            }
        }
        if (maxF <= 1) break;
        normFreqs[bestSym]--;
        totalNorm--;
    }

    // Build symbol position storage & encode table
    uint32_t symbolPosOffset[TANS4_MAX_SYMBOLS];
    uint32_t curOffset = 0;
    TANS4_EncSymbol encTable[TANS4_MAX_SYMBOLS];
    memset(encTable, 0, sizeof(encTable));

    for (uint32_t s = 0; s <= maxSymbol; ++s) {
        symbolPosOffset[s] = curOffset;
        uint16_t f         = normFreqs[s];
        if (f > 0) {
            encTable[s].freq   = f;
            encTable[s].k_base = (uint16_t)((int)tableLog - floorLog2((uint32_t)f));
            encTable[s].offset = curOffset;
            curOffset += f;
        }
    }

    uint16_t symbolPosStorage[TANS4_L_DEFAULT];
    uint32_t fillCount[TANS4_MAX_SYMBOLS];
    memset(fillCount, 0, sizeof(fillCount));

    uint32_t step = (L >> 1) + (L >> 3) + 3;
    uint32_t pos  = 0;

    for (uint32_t s = 0; s <= maxSymbol; ++s) {
        for (uint32_t i = 0; i < normFreqs[s]; ++i) {
            symbolPosStorage[symbolPosOffset[s] + fillCount[s]++] = (uint16_t)pos;
            pos = (pos + step) & (L - 1);
        }
    }

    size_t headerSize = 6 + 1 + 2 + (maxSymbol + 1) * 2 + 16 + 8;
    if (headerSize + srcSize > dstCapacity) {
        return 0;
    }

    size_t streamCap = (srcSize / 4) + 256;
    uint8_t* streamBufs[4];
    for (int j = 0; j < 4; ++j) {
        streamBufs[j] = (uint8_t*)malloc(streamCap);
        if (streamBufs[j] == NULL) {
            for (int k = 0; k < j; ++k) free(streamBufs[k]);
            return 0;
        }
    }

    ZS_BitCStreamBF writer[4];
    for (int j = 0; j < 4; ++j) {
        writer[j] = ZS_BitCStreamBF_init(streamBufs[j], streamCap);
    }

    uint32_t state[4] = { L, L, L, L };

    size_t subLen[4];
    subLen[0] = (srcSize + 3) / 4;
    subLen[1] = (srcSize + 2) / 4;
    subLen[2] = (srcSize + 1) / 4;
    subLen[3] = srcSize / 4;

    for (int j = 0; j < 4; ++j) {
        if (subLen[j] == 0) continue;
        size_t lastIdx = (subLen[j] - 1) * 4 + (size_t)j;
        for (ssize_t idx = (ssize_t)lastIdx; idx >= (ssize_t)j; idx -= 4) {
            uint8_t s = bytes[idx];
            TANS4_EncSymbol const* es = &encTable[s];
            uint32_t k = es->k_base;
            if ((state[j] >> k) < es->freq) {
                k--;
            }
            uint32_t val = state[j] & ((1U << k) - 1);
            ZS_BitCStreamBF_write(&writer[j], val, k);
            ZS_BitCStreamBF_flush(&writer[j]);
            uint32_t x_sh = (state[j] >> k) - es->freq;
            state[j] = L + symbolPosStorage[es->offset + x_sh];
        }
    }

    size_t streamSize[4];
    const uint8_t* streamData[4];
    size_t totalCompressedData = 0;

    for (int j = 0; j < 4; ++j) {
        ZL_Report rep = ZS_BitCStreamBF_finish(&writer[j]);
        if (ZL_isError(rep)) {
            for (int k = 0; k < 4; ++k) free(streamBufs[k]);
            return 0;
        }
        streamSize[j] = ZL_validResult(rep);
        streamData[j] = writer[j].ptr;
        totalCompressedData += streamSize[j];
    }

    if (headerSize + totalCompressedData >= srcSize) {
        for (int j = 0; j < 4; ++j) free(streamBufs[j]);
        if (dstCapacity < 6 + srcSize) return 0;
        out[5] = TANS4_MODE_UNCOMPRESSED;
        memcpy(out + 6, src, srcSize);
        return 6 + srcSize;
    }

    out[5] = TANS4_MODE_COMPRESSED;
    uint8_t* hPtr = out + 6;
    *hPtr++ = (uint8_t)tableLog;
    writeLE16(hPtr, (uint16_t)maxSymbol);
    hPtr += 2;

    for (uint32_t s = 0; s <= maxSymbol; ++s) {
        writeLE16(hPtr, normFreqs[s]);
        hPtr += 2;
    }

    for (int j = 0; j < 4; ++j) {
        writeLE32(hPtr, (uint32_t)streamSize[j]);
        hPtr += 4;
    }

    for (int j = 0; j < 4; ++j) {
        writeLE16(hPtr, (uint16_t)(state[j] - L));
        hPtr += 2;
    }

    for (int j = 0; j < 4; ++j) {
        if (streamSize[j] > 0) {
            memcpy(hPtr, streamData[j], streamSize[j]);
            hPtr += streamSize[j];
        }
        free(streamBufs[j]);
    }

    return (size_t)(hPtr - out);
}
