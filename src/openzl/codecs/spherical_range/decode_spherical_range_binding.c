// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "decode_spherical_range_binding.h"
#include "decode_spherical_range_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/zl_data.h"

ZL_Report DI_spherical_range(
        ZL_Decoder* dictx,
        const ZL_Input* ins[])
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(dictx);
    ZL_ASSERT_NN(dictx);
    ZL_ASSERT_NN(ins);

    const ZL_Input* const in = ins[0];
    ZL_ASSERT_NN(in);

    ZL_RBuffer const header = ZL_Decoder_getCodecHeader(dictx);
    ZL_ERR_IF_LE(header.size, 0, corruption, "spherical_range requires a codec header");
    size_t const intWidth = ((const uint8_t*)header.start)[0];
    ZL_ASSERT(intWidth == 1 || intWidth == 2 || intWidth == 4 || intWidth == 8);

    size_t const nbElts = ZL_Input_numElts(in);

    ZL_Output* const out = ZL_Decoder_create1OutStream(dictx, nbElts, intWidth);
    ZL_ERR_IF_NULL(out, allocation);

    const void* src = ZL_Input_ptr(in);
    void* dst = ZL_Output_ptr(out);

    ZS_sphericalRangeDecode(dst, src, nbElts, intWidth);

    ZL_ERR_IF_ERR(ZL_Output_commit(out, nbElts));

    return ZL_returnValue(1);
}
