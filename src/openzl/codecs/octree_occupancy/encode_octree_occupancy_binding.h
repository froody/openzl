// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_OCTREE_OCCUPANCY_ENCODE_OCTREE_OCCUPANCY_BINDING_H
#define OPENZL_CODECS_OCTREE_OCCUPANCY_ENCODE_OCTREE_OCCUPANCY_BINDING_H

#include "openzl/codecs/common/graph_pipe.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

ZL_Report EI_octree_occupancy(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns);

#define EI_OCTREE_OCCUPANCY(id) \
    { .gd          = PIPE_GRAPH(id), \
      .transform_f = EI_octree_occupancy, \
      .name        = "!zl.octree_occupancy" }

ZL_END_C_DECLS

#endif // OPENZL_CODECS_OCTREE_OCCUPANCY_ENCODE_OCTREE_OCCUPANCY_BINDING_H
