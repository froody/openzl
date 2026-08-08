// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_OCTREE_OCCUPANCY_DECODE_OCTREE_OCCUPANCY_BINDING_H
#define OPENZL_CODECS_OCTREE_OCCUPANCY_DECODE_OCTREE_OCCUPANCY_BINDING_H

#include "openzl/codecs/common/graph_pipe.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_dtransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

ZL_Report DI_octree_occupancy(ZL_Decoder* dictx, const ZL_Input* ins[]);

#define DI_OCTREE_OCCUPANCY(id) \
    { .gd = { .CTid = id, .inStreamType = ZL_Type_serial, .outStreamTypes = (const ZL_Type[]){ ZL_Type_serial }, .nbOutStreams = 1 }, \
      .transform_f = DI_octree_occupancy, \
      .name        = "!zl.octree_occupancy" }

ZL_END_C_DECLS

#endif // OPENZL_CODECS_OCTREE_OCCUPANCY_DECODE_OCTREE_OCCUPANCY_BINDING_H
