// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_LINEAR_PREDICT_H
#define ZSTRONG_CODECS_LINEAR_PREDICT_H

#include <stdint.h>

#include "openzl/zl_nodes.h"
#include "openzl/zl_opaque_types.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Inter-Ring & Intra-Neighbor Linear Residual Predictor
// Input  : 1 numeric stream (supported widths: 1, 2, 4, 8 bytes)
// Output : 1 numeric stream of residuals (same element width as input)
//
// Predicts point attributes or coordinates using linear combinations of spatial
// and ring neighbors (e.g. p_i - hat_p_i) and reconstructs original values losslessly.
#define ZL_NODE_LINEAR_PREDICT ZL_MAKE_NODE_ID(ZL_StandardNodeID_linear_predict)

/// Optional local param keys
/// 8-byte uint64_t stride (ring / neighbor distance S >= 1, default = 1)
#define ZL_LINEAR_PREDICT_STRIDE_PID 133

/// 8-byte uint64_t prediction mode (0: Planar, 1: Ring Delta, 2: 2nd-order, 3: Average, default = 0)
#define ZL_LINEAR_PREDICT_MODE_PID 134

#if defined(__cplusplus)
}
#endif

#endif // ZSTRONG_CODECS_LINEAR_PREDICT_H
