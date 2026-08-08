// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/octree_occupancy/encode_octree_occupancy_binding.h"
#include "openzl/codecs/octree_occupancy/encode_octree_occupancy_kernel.h"
#include "openzl/common/assertion.h"
#include "openzl/zl_data.h"
#include "openzl/zl_errors.h"

ZL_Report EI_octree_occupancy(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns)
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(eictx);
    ZL_ASSERT_NN(eictx);
    ZL_ASSERT_EQ(nbIns, 1);
    ZL_ASSERT_NN(ins);

    const ZL_Input* const in = ins[0];
    ZL_ASSERT_NN(in);

    const void* const src = ZL_Input_ptr(in);
    size_t const srcSize = ZL_Input_numElts(in) * ZL_Input_eltWidth(in);

    size_t const bound = ZS_octreeOccupancy_encodeBound(srcSize);
    ZL_Output* const out = ZL_Encoder_createTypedStream(eictx, 0, bound, 1);
    ZL_ERR_IF_NULL(out, allocation);

    size_t const written = ZS_octreeOccupancy_encode(ZL_Output_ptr(out), bound, src, srcSize);
    ZL_ERR_IF_EQ(written, 0, GENERIC, "Octree occupancy encode failed");

    ZL_ERR_IF_ERR(ZL_Output_commit(out, written));
    return ZL_returnSuccess();
}
