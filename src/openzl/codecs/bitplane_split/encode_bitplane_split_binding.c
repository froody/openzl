// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "encode_bitplane_split_binding.h"
#include "encode_bitplane_split_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/zl_data.h"

ZL_Report EI_bitplane_split(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns)
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

    uint8_t msbBits = (uint8_t)((eltWidth * 8) / 2);
    uint8_t header[2];
    header[0] = (uint8_t)eltWidth;
    header[1] = msbBits;
    ZL_Encoder_sendCodecHeader(eictx, header, 2);

    size_t const msbWidth = ZS_bitplaneSplit_getMsbWidth(eltWidth, msbBits);
    size_t const lsbWidth = ZS_bitplaneSplit_getLsbWidth(eltWidth, msbBits);

    ZL_Output* const outMsb = ZL_Encoder_createTypedStream(eictx, 0, nbElts, msbWidth);
    ZL_ERR_IF_NULL(outMsb, allocation);

    ZL_Output* const outLsb = ZL_Encoder_createTypedStream(eictx, 1, nbElts, lsbWidth);
    ZL_ERR_IF_NULL(outLsb, allocation);

    const void* src = ZL_Input_ptr(in);
    void* dstMsb = ZL_Output_ptr(outMsb);
    void* dstLsb = ZL_Output_ptr(outLsb);

    ZS_bitplaneSplitEncode(dstMsb, dstLsb, src, nbElts, eltWidth, msbBits);

    ZL_ERR_IF_ERR(ZL_Output_commit(outMsb, nbElts));
    ZL_ERR_IF_ERR(ZL_Output_commit(outLsb, nbElts));

    return ZL_returnValue(2);
}
