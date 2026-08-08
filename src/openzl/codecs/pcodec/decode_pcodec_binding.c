// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/pcodec/decode_pcodec_binding.h"
#include "openzl/codecs/pcodec/decode_pcodec_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/shared/varint.h"
#include "openzl/zl_data.h"

ZL_Report DI_pcodec_int(ZL_Decoder* dictx, const ZL_Input* ins[])
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(dictx);
    ZL_ASSERT_NN(dictx);
    ZL_ASSERT_NN(ins);

    const ZL_Input* const inLatents   = ins[0];
    const ZL_Input* const inResiduals = ins[1];
    ZL_ASSERT_NN(inLatents);
    ZL_ASSERT_NN(inResiduals);

    ZL_RBuffer const header = ZL_Decoder_getCodecHeader(dictx);
    ZL_ERR_IF_LE(header.size, 0, corruption, "pcodec requires a codec header");

    const uint8_t* ptr = (const uint8_t*)header.start;
    const uint8_t* end = ptr + header.size;

    size_t const eltWidth = *ptr++;
    ZL_ASSERT(eltWidth == 1 || eltWidth == 2 || eltWidth == 4 || eltWidth == 8);

    ZL_RESULT_OF(uint64_t) baseRes = ZL_varintDecode(&ptr, end);
    ZL_TRY_LET(uint64_t, baseOffset, baseRes);

    ZL_RESULT_OF(uint64_t) multRes = ZL_varintDecode(&ptr, end);
    ZL_TRY_LET(uint64_t, multiplier, multRes);

    size_t const nbLatents   = ZL_Input_numElts(inLatents);
    size_t const nbResiduals = ZL_Input_numElts(inResiduals);
    ZL_ERR_IF_NE(nbLatents, nbResiduals, corruption, "pcodec latent and residual sizes must match");

    ZL_Output* const out = ZL_Decoder_create1OutStream(dictx, nbLatents, eltWidth);
    ZL_ERR_IF_NULL(out, allocation);

    const void* srcLatents   = ZL_Input_ptr(inLatents);
    const void* srcResiduals = ZL_Input_ptr(inResiduals);
    void* dst                = ZL_Output_ptr(out);

    ZS_pcodecDecode(dst, srcLatents, srcResiduals, nbLatents, eltWidth, baseOffset, multiplier);

    ZL_ERR_IF_ERR(ZL_Output_commit(out, nbLatents));

    return ZL_returnValue(1);
}
