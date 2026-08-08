// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_OCTREE_OCCUPANCY_H
#define ZSTRONG_CODECS_OCTREE_OCCUPANCY_H

#include "openzl/zl_nodes.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Octree Occupancy Byte-Mask Encoding Transform
// Input  : 1 serial stream (3D point cloud)
// Output : 1 serial stream (octree occupancy byte-masks + metadata)
#define ZL_NODE_OCTREE_OCCUPANCY ZL_MAKE_NODE_ID(ZL_StandardNodeID_octree_occupancy)

#if defined(__cplusplus)
}
#endif

#endif // ZSTRONG_CODECS_OCTREE_OCCUPANCY_H
