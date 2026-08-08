// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "encode_linear_predict_binding.h"
#include "encode_linear_predict_kernel.h"
#include "openzl/codecs/zl_linear_predict.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/shared/varint.h"
#include "openzl/zl_data.h"

ZL_Report EI_linear_predict(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns)
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(eictx);
    ZL_ASSERT_NN(eictx);
    ZL_ASSERT_EQ(nbIns, 1);
    ZL_ASSERT_NN(ins);
    const ZL_Input* in = ins[0];
    ZL_ASSERT_NN(in);
    ZL_ASSERT_EQ(ZL_Input_type(in), ZL_Type_numeric);
    size_t const intWidth = ZL_Input_eltWidth(in);
    ZL_ASSERT(intWidth == 1 || intWidth == 2 || intWidth == 4 || intWidth == 8);
    size_t const nbElts = ZL_Input_numElts(in);

    size_t stride = 1;
    ZL_RefParam strideParam = ZL_Encoder_getLocalParam(eictx, ZL_LINEAR_PREDICT_STRIDE_PID);
    if (strideParam.paramRef != NULL) {
        ZL_ERR_IF_NE(strideParam.paramSize, sizeof(uint64_t), graphParameter_invalid);
        uint64_t const val = *(const uint64_t*)strideParam.paramRef;
        if (val > 0) {
            stride = (size_t)val;
        }
    }

    uint8_t mode = 0;
    ZL_RefParam modeParam = ZL_Encoder_getLocalParam(eictx, ZL_LINEAR_PREDICT_MODE_PID);
    if (modeParam.paramRef != NULL) {
        ZL_ERR_IF_NE(modeParam.paramSize, sizeof(uint64_t), graphParameter_invalid);
        uint64_t const val = *(const uint64_t*)modeParam.paramRef;
        mode = (uint8_t)(val & 0xFF);
    }

    uint8_t header[2 + ZL_VARINT_LENGTH_64];
    header[0] = (uint8_t)intWidth;
    header[1] = mode;
    size_t const varintLen = ZL_varintEncode((uint64_t)stride, header + 2);
    ZL_Encoder_sendCodecHeader(eictx, header, 2 + varintLen);

    ZL_Output* const out = ZL_Encoder_createTypedStream(eictx, 0, nbElts, intWidth);
    ZL_ERR_IF_NULL(out, allocation);

    const void* src = ZL_Input_ptr(in);
    void* dst = ZL_Output_ptr(out);

    ZS_linearPredictEncode(dst, src, nbElts, intWidth, stride, mode);

    ZL_ERR_IF_ERR(ZL_Output_commit(out, nbElts));

    return ZL_returnValue(1);
}
