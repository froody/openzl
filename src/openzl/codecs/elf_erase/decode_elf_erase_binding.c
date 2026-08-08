// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/elf_erase/decode_elf_erase_binding.h"
#include "openzl/codecs/elf_erase/decode_elf_erase_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/shared/varint.h"
#include "openzl/zl_data.h"

ZL_Report DI_elf_erase(ZL_Decoder* dictx, const ZL_Input* ins[])
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(dictx);
    ZL_ASSERT_NN(dictx);
    ZL_ASSERT_NN(ins);

    const ZL_Input* const in = ins[0];
    ZL_ASSERT_NN(in);

    ZL_RBuffer const header = ZL_Decoder_getCodecHeader(dictx);
    ZL_ERR_IF_LE(header.size, 0, corruption, "elf_erase requires a codec header");

    const uint8_t* ptr = (const uint8_t*)header.start;
    const uint8_t* end = ptr + header.size;

    size_t const eltWidth = *ptr++;
    ZL_ASSERT(eltWidth == 1 || eltWidth == 2 || eltWidth == 4 || eltWidth == 8);

    ZL_RESULT_OF(uint64_t) nbEltsRes = ZL_varintDecode(&ptr, end);
    ZL_TRY_LET(uint64_t, nbElts, nbEltsRes);

    ZL_Output* const out = ZL_Decoder_create1OutStream(dictx, nbElts, eltWidth);
    ZL_ERR_IF_NULL(out, allocation);

    const void* src = ZL_Input_ptr(in);
    size_t const srcSize = ZL_Input_numElts(in);
    void* dst = ZL_Output_ptr(out);

    if (nbElts > 0) {
        int const ok = ZS_elfEraseDecode(dst, src, srcSize, nbElts, eltWidth);
        ZL_ERR_IF(!ok, corruption, "elf_erase decoding failed");
    }

    ZL_ERR_IF_ERR(ZL_Output_commit(out, nbElts));
    return ZL_returnValue(1);
}
