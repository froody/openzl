// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/morton_3d/encode_morton_3d_binding.h"
#include "openzl/codecs/morton_3d/encode_morton_3d_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/zl_data.h"

ZL_Report EI_morton_3d(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns)
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(eictx);
    ZL_ASSERT_NN(eictx);
    ZL_ASSERT_NN(ins);
    ZL_ERR_IF(
            nbIns != 1 && nbIns != 3,
            node_invalid_input,
            "morton_3d expects either 3 input streams (X, Y, Z) or 1 interleaved stream");

    if (nbIns == 3) {
        const ZL_Input* inX = ins[0];
        const ZL_Input* inY = ins[1];
        const ZL_Input* inZ = ins[2];
        ZL_ASSERT_NN(inX);
        ZL_ASSERT_NN(inY);
        ZL_ASSERT_NN(inZ);

        ZL_ASSERT_EQ(ZL_Input_type(inX), ZL_Type_numeric);
        ZL_ASSERT_EQ(ZL_Input_type(inY), ZL_Type_numeric);
        ZL_ASSERT_EQ(ZL_Input_type(inZ), ZL_Type_numeric);

        size_t const coordWidth = ZL_Input_eltWidth(inX);
        ZL_ASSERT(coordWidth == 1 || coordWidth == 2 || coordWidth == 4 || coordWidth == 8);
        ZL_ERR_IF_NE(
                ZL_Input_eltWidth(inY),
                coordWidth,
                node_invalid_input,
                "morton_3d inputs must have identical element widths");
        ZL_ERR_IF_NE(
                ZL_Input_eltWidth(inZ),
                coordWidth,
                node_invalid_input,
                "morton_3d inputs must have identical element widths");

        size_t const nbElts = ZL_Input_numElts(inX);
        ZL_ERR_IF_NE(
                ZL_Input_numElts(inY),
                nbElts,
                node_invalid_input,
                "morton_3d inputs must have identical element counts");
        ZL_ERR_IF_NE(
                ZL_Input_numElts(inZ),
                nbElts,
                node_invalid_input,
                "morton_3d inputs must have identical element counts");

        uint8_t header[2];
        header[0] = (uint8_t)coordWidth;
        header[1] = 0; // 0 = 3 separate input streams
        ZL_Encoder_sendCodecHeader(eictx, header, 2);

        ZL_Output* const out =
                ZL_Encoder_createTypedStream(eictx, 0, nbElts, sizeof(uint64_t));
        ZL_ERR_IF_NULL(out, allocation);

        ZS_morton3dEncode(
                (uint64_t*)ZL_Output_ptr(out),
                ZL_Input_ptr(inX),
                ZL_Input_ptr(inY),
                ZL_Input_ptr(inZ),
                nbElts,
                coordWidth);

        ZL_ERR_IF_ERR(ZL_Output_commit(out, nbElts));
        return ZL_returnValue(1);
    } else {
        const ZL_Input* in = ins[0];
        ZL_ASSERT_NN(in);
        ZL_ASSERT_EQ(ZL_Input_type(in), ZL_Type_numeric);

        size_t const coordWidth = ZL_Input_eltWidth(in);
        ZL_ASSERT(coordWidth == 1 || coordWidth == 2 || coordWidth == 4 || coordWidth == 8);

        size_t const totalElts = ZL_Input_numElts(in);
        ZL_ERR_IF_NE(
                totalElts % 3,
                0,
                node_invalid_input,
                "interleaved morton_3d input count must be divisible by 3");

        size_t const nbElts = totalElts / 3;

        uint8_t header[2];
        header[0] = (uint8_t)coordWidth;
        header[1] = 1; // 1 = 1 interleaved input stream
        ZL_Encoder_sendCodecHeader(eictx, header, 2);

        ZL_Output* const out =
                ZL_Encoder_createTypedStream(eictx, 0, nbElts, sizeof(uint64_t));
        ZL_ERR_IF_NULL(out, allocation);

        ZS_morton3dEncodeInterleaved(
                (uint64_t*)ZL_Output_ptr(out),
                ZL_Input_ptr(in),
                nbElts,
                coordWidth);

        ZL_ERR_IF_ERR(ZL_Output_commit(out, nbElts));
        return ZL_returnValue(1);
    }
}
