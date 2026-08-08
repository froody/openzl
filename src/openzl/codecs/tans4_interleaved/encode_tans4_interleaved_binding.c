// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/tans4_interleaved/encode_tans4_interleaved_binding.h"
#include "openzl/codecs/tans4_interleaved/encode_tans4_interleaved_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/zl_data.h"

ZL_Report EI_tans4_interleaved(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns)
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(eictx);
    ZL_ASSERT_NN(eictx);
    ZL_ASSERT_EQ(nbIns, 1);
    ZL_ASSERT_NN(ins);

    const ZL_Input* in = ins[0];
    ZL_ASSERT_NN(in);

    const void* src = ZL_Input_ptr(in);
    size_t srcSize  = ZL_Input_numElts(in);

    size_t bound = ZS_tans4EncodeBound(srcSize);
    ZL_Output* const out = ZL_Encoder_createTypedStream(eictx, 0, bound, 1);
    ZL_ERR_IF_NULL(out, allocation);

    size_t cSize = ZS_tans4Encode(ZL_Output_ptr(out), bound, src, srcSize);
    ZL_ERR_IF_EQ(cSize, 0, GENERIC, "tans4_interleaved encode failed");

    ZL_ERR_IF_ERR(ZL_Output_commit(out, cSize));

    return ZL_returnSuccess();
}
