// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/elf_erase/encode_elf_erase_binding.h"
#include "openzl/codecs/elf_erase/encode_elf_erase_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/shared/varint.h"
#include "openzl/zl_data.h"

ZL_Report EI_elf_erase(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns)
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(eictx);
    ZL_ASSERT_NN(eictx);
    ZL_ASSERT_EQ(nbIns, 1);
    ZL_ASSERT_NN(ins);
    const ZL_Input* in = ins[0];
    ZL_ASSERT_NN(in);
    ZL_ASSERT_EQ(ZL_Input_type(in), ZL_Type_numeric);

    size_t const eltWidth = ZL_Input_eltWidth(in);
    ZL_ASSERT(eltWidth == 1 || eltWidth == 2 || eltWidth == 4 || eltWidth == 8);
    size_t const nbElts = ZL_Input_numElts(in);

    uint8_t header[16];
    header[0] = (uint8_t)eltWidth;
    size_t hLen = 1 + ZL_varintEncode64Fast(nbElts, header + 1);
    ZL_Encoder_sendCodecHeader(eictx, header, hLen);

    size_t const maxOutputCapacity = nbElts * (eltWidth + 4) + 64;
    ZL_Output* const out = ZL_Encoder_createTypedStream(eictx, 0, maxOutputCapacity, 1);
    ZL_ERR_IF_NULL(out, allocation);

    void* const dst = ZL_Output_ptr(out);
    const void* const src = ZL_Input_ptr(in);

    size_t bytesWritten = 0;
    if (nbElts > 0) {
        bytesWritten = ZS_elfEraseEncode(dst, maxOutputCapacity, src, nbElts, eltWidth);
        ZL_ERR_IF_EQ(bytesWritten, 0, transform_executionFailure, "elf_erase encoding failed");
    }

    ZL_ERR_IF_ERR(ZL_Output_commit(out, bytesWritten));
    return ZL_returnValue(1);
}
