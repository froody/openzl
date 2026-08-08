// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/tans4_interleaved/decode_tans4_interleaved_binding.h"
#include "openzl/codecs/tans4_interleaved/decode_tans4_interleaved_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/zl_data.h"

ZL_Report DI_tans4_interleaved(ZL_Decoder* dictx, const ZL_Input* ins[])
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(dictx);
    ZL_ASSERT_NN(dictx);
    ZL_ASSERT_NN(ins);

    const ZL_Input* const in = ins[0];
    ZL_ASSERT_NN(in);

    const uint8_t* src = (const uint8_t*)ZL_Input_ptr(in);
    size_t srcSize    = ZL_Input_numElts(in);

    size_t decompressedSize = ZS_tans4GetDecompressedSize(src, srcSize);

    ZL_Output* const out = ZL_Decoder_createTypedStream(dictx, 0, decompressedSize, 1);
    ZL_ERR_IF_NULL(out, allocation);

    size_t decoded = ZS_tans4Decode(ZL_Output_ptr(out), decompressedSize, src, srcSize, decompressedSize);
    ZL_ERR_IF_NE(decoded, decompressedSize, corruption, "tans4_interleaved decode failed");

    ZL_ERR_IF_ERR(ZL_Output_commit(out, decompressedSize));

    return ZL_returnSuccess();
}
