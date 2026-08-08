// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "decode_iwt_binding.h"
#include "decode_iwt_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/zl_data.h"

ZL_Report DI_iwt_int(ZL_Decoder* dictx, const ZL_Input* ins[])
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(dictx);
    ZL_ASSERT_NN(dictx);
    ZL_ASSERT_NN(ins);

    const ZL_Input* const inApprox = ins[0];
    const ZL_Input* const inDetail = ins[1];
    ZL_ASSERT_NN(inApprox);
    ZL_ASSERT_NN(inDetail);

    ZL_RBuffer const header = ZL_Decoder_getCodecHeader(dictx);
    ZL_ERR_IF_LE(header.size, 0, corruption, "iwt requires a codec header");
    size_t const intWidth = ((const uint8_t*)header.start)[0];
    ZL_ASSERT(intWidth == 1 || intWidth == 2 || intWidth == 4 || intWidth == 8);

    size_t const nbApprox = ZL_Input_numElts(inApprox);
    size_t const nbDetail = ZL_Input_numElts(inDetail);
    size_t const totalElts = nbApprox + nbDetail;

    ZL_Output* const out = ZL_Decoder_create1OutStream(dictx, totalElts, intWidth);
    ZL_ERR_IF_NULL(out, allocation);

    const void* srcApprox = ZL_Input_ptr(inApprox);
    const void* srcDetail = ZL_Input_ptr(inDetail);
    void* dst = ZL_Output_ptr(out);

    ZS_iwtDecode(dst, srcApprox, srcDetail, totalElts, intWidth);

    ZL_ERR_IF_ERR(ZL_Output_commit(out, totalElts));

    return ZL_returnValue(1);
}
