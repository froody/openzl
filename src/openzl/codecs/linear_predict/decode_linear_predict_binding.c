// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "decode_linear_predict_binding.h"
#include "decode_linear_predict_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/shared/varint.h"
#include "openzl/zl_data.h"

ZL_Report DI_linear_predict(ZL_Decoder* dictx, const ZL_Input* ins[])
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(dictx);
    ZL_ASSERT_NN(dictx);
    ZL_ASSERT_NN(ins);

    const ZL_Input* const in = ins[0];
    ZL_ASSERT_NN(in);

    ZL_RBuffer const header = ZL_Decoder_getCodecHeader(dictx);
    ZL_ERR_IF_LT(header.size, 3, corruption, "linear_predict requires a codec header of at least 3 bytes");

    const uint8_t* ptr = (const uint8_t*)header.start;
    const uint8_t* end = ptr + header.size;

    size_t const intWidth = ptr[0];
    ZL_ERR_IF(!(intWidth == 1 || intWidth == 2 || intWidth == 4 || intWidth == 8), corruption, "Invalid element width");

    uint8_t const mode = ptr[1];

    ptr += 2;
    ZL_TRY_LET(uint64_t, strideVal, ZL_varintDecode64Strict(&ptr, end));
    ZL_ERR_IF_EQ(strideVal, 0, corruption, "Invalid stride 0");
    size_t const stride = (size_t)strideVal;

    size_t const nbElts = ZL_Input_numElts(in);
    ZL_Output* const out = ZL_Decoder_create1OutStream(dictx, nbElts, intWidth);
    ZL_ERR_IF_NULL(out, allocation);

    const void* src = ZL_Input_ptr(in);
    void* dst = ZL_Output_ptr(out);

    ZS_linearPredictDecode(dst, src, nbElts, intWidth, stride, mode);

    ZL_ERR_IF_ERR(ZL_Output_commit(out, nbElts));

    return ZL_returnValue(1);
}
