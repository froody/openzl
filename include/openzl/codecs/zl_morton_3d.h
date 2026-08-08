// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_MORTON_3D_H
#define OPENZL_CODECS_MORTON_3D_H

#include "openzl/zl_nodes.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Morton 3D Z-Order Curve Spatial Reordering Transform
// Input  (Encode): 3 numeric streams (X, Y, Z coordinates; width: 1, 2, 4, or 8 bytes)
// Output (Encode): 1 numeric stream of 64-bit Morton Z-order codes (width: 8 bytes)
// Input  (Decode): 1 numeric stream of 64-bit Morton Z-order codes (width: 8 bytes)
// Output (Decode): 3 numeric streams (X, Y, Z coordinates; width: 1, 2, 4, or 8 bytes)
#define ZL_NODE_MORTON_3D ZL_MAKE_NODE_ID(ZL_StandardNodeID_morton_3d)

#if defined(__cplusplus)
}
#endif

#endif // OPENZL_CODECS_MORTON_3D_H
