// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/morton_3d/decode_morton_3d_binding.h"
#include "openzl/codecs/morton_3d/decode_morton_3d_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/zl_data.h"

ZL_Report DI_morton_3d(ZL_Decoder* dictx, const ZL_Input* ins[])
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(dictx);
    ZL_ASSERT_NN(dictx);
    ZL_ASSERT_NN(ins);

    const ZL_Input* const in = ins[0];
    ZL_ASSERT_NN(in);
    ZL_ASSERT_EQ(ZL_Input_type(in), ZL_Type_numeric);

    ZL_RBuffer const header = ZL_Decoder_getCodecHeader(dictx);
    ZL_ERR_IF_LE(header.size, 0, corruption, "morton_3d requires codec header");

    size_t const coordWidth    = ((const uint8_t*)header.start)[0];
    uint8_t const inputFormat  = (header.size >= 2) ? ((const uint8_t*)header.start)[1] : 0;

    ZL_ASSERT(coordWidth == 1 || coordWidth == 2 || coordWidth == 4 || coordWidth == 8);

    size_t const nbElts = ZL_Input_numElts(in);
    const uint64_t* mortonCodes = (const uint64_t*)ZL_Input_ptr(in);

    if (inputFormat == 0) {
        ZL_Output* const outX = ZL_Decoder_createTypedStream(dictx, 0, nbElts, coordWidth);
        ZL_ERR_IF_NULL(outX, allocation);

        ZL_Output* const outY = ZL_Decoder_createTypedStream(dictx, 1, nbElts, coordWidth);
        ZL_ERR_IF_NULL(outY, allocation);

        ZL_Output* const outZ = ZL_Decoder_createTypedStream(dictx, 2, nbElts, coordWidth);
        ZL_ERR_IF_NULL(outZ, allocation);

        ZS_morton3dDecode(
                ZL_Output_ptr(outX),
                ZL_Output_ptr(outY),
                ZL_Output_ptr(outZ),
                mortonCodes,
                nbElts,
                coordWidth);

        ZL_ERR_IF_ERR(ZL_Output_commit(outX, nbElts));
        ZL_ERR_IF_ERR(ZL_Output_commit(outY, nbElts));
        ZL_ERR_IF_ERR(ZL_Output_commit(outZ, nbElts));

        return ZL_returnValue(3);
    } else {
        size_t const totalElts = nbElts * 3;
        ZL_Output* const out = ZL_Decoder_createTypedStream(dictx, 0, totalElts, coordWidth);
        ZL_ERR_IF_NULL(out, allocation);

        ZS_morton3dDecodeInterleaved(
                ZL_Output_ptr(out),
                mortonCodes,
                nbElts,
                coordWidth);

        ZL_ERR_IF_ERR(ZL_Output_commit(out, totalElts));
        return ZL_returnValue(1);
    }
}
