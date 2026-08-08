// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_SPHERICAL_RANGE_H
#define ZSTRONG_CODECS_SPHERICAL_RANGE_H

#include "openzl/zl_nodes.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Spherical Range Image Projection Transform
// Input  : 1 numeric stream of 3D Cartesian coordinates (x, y, z)
// Output : 1 numeric stream of fixed-point spherical coordinates (r, theta, phi)
#define ZL_NODE_SPHERICAL_RANGE ZL_MAKE_NODE_ID(ZL_StandardNodeID_spherical_range)

#if defined(__cplusplus)
}
#endif

#endif // ZSTRONG_CODECS_SPHERICAL_RANGE_H
