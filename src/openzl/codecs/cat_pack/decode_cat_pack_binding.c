// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/cat_pack/decode_cat_pack_binding.h"
#include "openzl/codecs/cat_pack/decode_cat_pack_kernel.h"

#include "openzl/common/errors_internal.h"
#include "openzl/shared/mem.h"
#include "openzl/zl_dtransform.h"
#include "openzl/zl_data.h"
#include "openzl/zl_errors.h"

ZL_Report DI_catPack(ZL_Decoder* dictx, const ZL_Input* ins[])
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(dictx);
    ZL_ASSERT_NN(ins);
    const ZL_Input* in    = ins[0];
    const void* src       = ZL_Input_ptr(in);
    size_t nbElts = ZL_Input_numElts(in);
    
    // cat_pack compresses N elements of 1 byte into exactly N elements of 1 byte.
    // The bits beyond the packed dict are zeroed out by the encoder, which was compressed by zstd.
    // At this point we have N bytes back.

    ZL_Output* const dstStream =
            ZL_Decoder_createTypedStream(dictx, 0, nbElts, 1);
    ZL_ERR_IF(!dstStream, allocation);
    void* dst = ZL_Output_ptr(dstStream);
    
    int ok = catPackDecode(dst, src, nbElts, nbElts);
    if (!ok) {
        ZL_ERR(corruption);
    }
    
    ZL_ERR_IF_ERR(ZL_Output_commit(dstStream, nbElts));

    return ZL_returnValue(1);
}
