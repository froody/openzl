// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "encode_spherical_range_binding.h"
#include "encode_spherical_range_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/zl_data.h"

ZL_Report EI_spherical_range(
        ZL_Encoder* eictx,
        const ZL_Input* ins[],
        size_t nbIns)
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

    uint8_t header[1];
    header[0] = (uint8_t)intWidth;
    ZL_Encoder_sendCodecHeader(eictx, header, 1);

    ZL_Output* const out = ZL_Encoder_createTypedStream(eictx, 0, nbElts, intWidth);
    ZL_ERR_IF_NULL(out, allocation);

    const void* src = ZL_Input_ptr(in);
    void* dst = ZL_Output_ptr(out);

    ZS_sphericalRangeEncode(dst, src, nbElts, intWidth);

    ZL_ERR_IF_ERR(ZL_Output_commit(out, nbElts));

    return ZL_returnValue(1);
}
