// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "decode_bitplane_split_binding.h"
#include "decode_bitplane_split_kernel.h"
#include "encode_bitplane_split_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/zl_data.h"

ZL_Report DI_bitplane_split(ZL_Decoder* dictx, const ZL_Input* ins[])
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(dictx);
    ZL_ASSERT_NN(dictx);
    ZL_ASSERT_NN(ins);

    const ZL_Input* const inMsb = ins[0];
    const ZL_Input* const inLsb = ins[1];
    ZL_ASSERT_NN(inMsb);
    ZL_ASSERT_NN(inLsb);

    ZL_RBuffer const header = ZL_Decoder_getCodecHeader(dictx);
    ZL_ERR_IF_LE(header.size, 0, corruption, "bitplane_split requires a codec header");
    const uint8_t* headerBytes = (const uint8_t*)header.start;
    size_t const eltWidth = headerBytes[0];
    ZL_ASSERT(eltWidth == 1 || eltWidth == 2 || eltWidth == 4 || eltWidth == 8);

    uint8_t msbBits = (header.size >= 2) ? headerBytes[1] : (uint8_t)((eltWidth * 8) / 2);
    if (msbBits == 0 || msbBits >= eltWidth * 8) {
        msbBits = (uint8_t)((eltWidth * 8) / 2);
    }

    size_t const nbElts = ZL_Input_numElts(inMsb);
    ZL_ERR_IF_NE(nbElts, ZL_Input_numElts(inLsb), corruption);

    ZL_Output* const out = ZL_Decoder_create1OutStream(dictx, nbElts, eltWidth);
    ZL_ERR_IF_NULL(out, allocation);

    const void* srcMsb = ZL_Input_ptr(inMsb);
    const void* srcLsb = ZL_Input_ptr(inLsb);
    void* dst = ZL_Output_ptr(out);

    ZS_bitplaneSplitDecode(dst, srcMsb, srcLsb, nbElts, eltWidth, msbBits);

    ZL_ERR_IF_ERR(ZL_Output_commit(out, nbElts));

    return ZL_returnValue(1);
}
