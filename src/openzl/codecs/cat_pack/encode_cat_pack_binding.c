// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/cat_pack/encode_cat_pack_binding.h"
#include "openzl/codecs/cat_pack/encode_cat_pack_kernel.h"

#include "openzl/common/errors_internal.h"
#include "openzl/shared/mem.h"
#include "openzl/zl_ctransform.h"
#include "openzl/zl_data.h"
#include "openzl/zl_errors.h"

ZL_Report EI_catPack(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns)
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(eictx);
    ZL_ASSERT_EQ(nbIns, 1);
    ZL_ASSERT_NN(ins);
    const ZL_Input* in    = ins[0];
    void const* src       = ZL_Input_ptr(in);
    size_t const srcWidth = ZL_Input_eltWidth(in);
    size_t const nbElts   = ZL_Input_numElts(in);
    
    // cat_pack currently only supports 8-bit widths.
    if (srcWidth != 1) {
        ZL_ERR(node_invalid_input);
    }

    // Max possible output size: 1 byte header + 16 bytes unique + nbElts bytes packed (uncompressed worst case length)
    // We allocate enough space to guarantee no overflow.
    size_t maxOutputSize = 1 + 16 + nbElts;
    if (nbElts > maxOutputSize) maxOutputSize = nbElts;

    ZL_Output* const dstStream =
            ZL_Encoder_createTypedStream(eictx, 0, maxOutputSize, 1);
    ZL_ERR_IF(!dstStream, allocation);
    void* dst = ZL_Output_ptr(dstStream);
    
    size_t compressed_len = catPackEncode(dst, src, nbElts);
    if (compressed_len == 0 || compressed_len >= nbElts) {
        // Failed (too many classes) or no compression benefit
        // Revert stream (handled implicitly when returning error)
        ZL_ERR(node_invalid_input);
    }
    
    // Zero-fill the remainder to optimize compression
    if (compressed_len < nbElts) {
        memset((uint8_t*)dst + compressed_len, 0, nbElts - compressed_len);
    }
    
    // We output exactly nbElts so the decoder knows how many elements there are!
    ZL_ERR_IF_ERR(ZL_Output_commit(dstStream, nbElts));

    // Send an empty header since we don't need any additional state
    ZL_Encoder_sendCodecHeader(eictx, NULL, 0);
    return ZL_returnValue(1);
}
