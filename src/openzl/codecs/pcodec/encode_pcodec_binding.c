// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/pcodec/encode_pcodec_binding.h"
#include "openzl/codecs/pcodec/encode_pcodec_kernel.h"
#include "openzl/codecs/zl_pcodec.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/shared/varint.h"
#include "openzl/zl_data.h"

static uint64_t computeBaseOffset(const void* src, size_t nbElts, size_t eltWidth)
{
    if (nbElts == 0) return 0;
    switch (eltWidth) {
        case 1: {
            const uint8_t* p = (const uint8_t*)src;
            uint8_t minVal = p[0];
            for (size_t i = 1; i < nbElts; ++i) if (p[i] < minVal) minVal = p[i];
            return minVal;
        }
        case 2: {
            const uint16_t* p = (const uint16_t*)src;
            uint16_t minVal = p[0];
            for (size_t i = 1; i < nbElts; ++i) if (p[i] < minVal) minVal = p[i];
            return minVal;
        }
        case 4: {
            const uint32_t* p = (const uint32_t*)src;
            uint32_t minVal = p[0];
            for (size_t i = 1; i < nbElts; ++i) if (p[i] < minVal) minVal = p[i];
            return minVal;
        }
        case 8: {
            const uint64_t* p = (const uint64_t*)src;
            uint64_t minVal = p[0];
            for (size_t i = 1; i < nbElts; ++i) if (p[i] < minVal) minVal = p[i];
            return minVal;
        }
        default: return 0;
    }
}

ZL_Report EI_pcodec_int(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns)
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(eictx);
    ZL_ASSERT_NN(eictx);
    ZL_ASSERT_EQ(nbIns, 1);
    ZL_ASSERT_NN(ins);
    const ZL_Input* in = ins[0];
    ZL_ASSERT_NN(in);
    ZL_ASSERT_EQ(ZL_Input_type(in), ZL_Type_numeric);
    size_t const eltWidth = ZL_Input_eltWidth(in);
    ZL_ASSERT(eltWidth == 1 || eltWidth == 2 || eltWidth == 4 || eltWidth == 8);
    size_t const nbElts = ZL_Input_numElts(in);

    const void* src = ZL_Input_ptr(in);

    uint64_t baseOffset = computeBaseOffset(src, nbElts, eltWidth);
    uint64_t multiplier = 1;

    ZL_RefParam param = ZL_Encoder_getLocalParam(eictx, ZL_PCODEC_PID);
    if (param.paramRef != NULL && param.paramSize == sizeof(uint64_t)) {
        multiplier = *(const uint64_t*)param.paramRef;
    }

    uint8_t header[ZL_VARINT_LENGTH_64 * 3];
    size_t headerSize = 0;
    header[headerSize++] = (uint8_t)eltWidth;
    headerSize += ZL_varintEncode64Fast(header + headerSize, baseOffset);
    headerSize += ZL_varintEncode64Fast(header + headerSize, multiplier);

    ZL_Encoder_sendCodecHeader(eictx, header, headerSize);

    ZL_Output* const out0 = ZL_Encoder_createTypedStream(eictx, 0, nbElts, eltWidth);
    ZL_ERR_IF_NULL(out0, allocation);

    ZL_Output* const out1 = ZL_Encoder_createTypedStream(eictx, 1, nbElts, eltWidth);
    ZL_ERR_IF_NULL(out1, allocation);

    void* dstLatents   = ZL_Output_ptr(out0);
    void* dstResiduals = ZL_Output_ptr(out1);

    ZS_pcodecEncode(dstLatents, dstResiduals, src, nbElts, eltWidth, baseOffset, multiplier);

    ZL_ERR_IF_ERR(ZL_Output_commit(out0, nbElts));
    ZL_ERR_IF_ERR(ZL_Output_commit(out1, nbElts));

    return ZL_returnValue(2);
}
