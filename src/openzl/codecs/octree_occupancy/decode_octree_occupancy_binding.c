// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/octree_occupancy/decode_octree_occupancy_binding.h"
#include "openzl/codecs/octree_occupancy/decode_octree_occupancy_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/zl_data.h"
#include "openzl/zl_errors.h"

ZL_Report DI_octree_occupancy(ZL_Decoder* dictx, const ZL_Input* ins[])
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(dictx);
    ZL_ASSERT_NN(dictx);
    ZL_ASSERT_NN(ins);

    const ZL_Input* const in = ins[0];
    ZL_ASSERT_NN(in);

    const void* const src = ZL_Input_ptr(in);
    size_t const srcSize = ZL_Input_numElts(in) * ZL_Input_eltWidth(in);

    size_t const decodedSize = ZS_octreeOccupancy_getDecodedSize(src, srcSize);
    ZL_ERR_IF_EQ(decodedSize, 0, corruption, "Invalid octree occupancy stream header");

    ZL_Output* const out = ZL_Decoder_create1OutStream(dictx, decodedSize, 1);
    ZL_ERR_IF_NULL(out, allocation);

    size_t const actualSize = ZS_octreeOccupancy_decode(ZL_Output_ptr(out), decodedSize, src, srcSize);
    ZL_ERR_IF_NE(actualSize, decodedSize, corruption, "Octree occupancy decode failed");

    ZL_ERR_IF_ERR(ZL_Output_commit(out, decodedSize));
    return ZL_returnSuccess();
}
