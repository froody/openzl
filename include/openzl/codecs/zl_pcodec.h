// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_PCODEC_H
#define ZSTRONG_CODECS_PCODEC_H

#include <stdint.h>

#include "openzl/zl_nodes.h"
#include "openzl/zl_opaque_types.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Pcodec (Pco) Numeric Sequence Pre-Conditioner
// Input  : 1 numeric stream (supported widths: 1, 2, 4, 8 bytes)
// Output : 2 numeric streams (same element width as input)
//          Output 0: Latent bin indices k_i = (x_i - B) / M
//          Output 1: Residual offsets r_i = (x_i - B) - k_i * M
#define ZL_NODE_PCODEC_INT ZL_MAKE_NODE_ID(ZL_StandardNodeID_pcodec_int)

/// Optional 8-byte local param key containing multiplier / bin step M.
/// If unset or 0, M is auto-detected via GCD / step analysis.
#define ZL_PCODEC_PID 113

/**
 * Registers Pcodec node with its multiplier parameter set to @p multiplier.
 */
ZL_NodeID ZL_Compressor_registerPcodecNode(
        ZL_Compressor* cgraph,
        uint64_t multiplier);

#if defined(__cplusplus)
}
#endif

#endif // ZSTRONG_CODECS_PCODEC_H
