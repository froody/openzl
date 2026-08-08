// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_IWT_H
#define ZSTRONG_CODECS_IWT_H

#include "openzl/zl_nodes.h"

#if defined(__cplusplus)
extern "C" {
#endif

// 1D Integer Wavelet Transform (5/3 LeGall Lifting Scheme)
// Input  : 1 numeric stream (all widths supported: 1, 2, 4, 8 bytes)
// Output : 2 numeric streams (same width as input)
//          Output 0: Low-pass / Approximation coefficients s (length ceil(N/2))
//          Output 1: High-pass / Detail coefficients d (length floor(N/2))
#define ZL_NODE_IWT_INT ZL_MAKE_NODE_ID(ZL_StandardNodeID_iwt_int)

#if defined(__cplusplus)
}
#endif

#endif // ZSTRONG_CODECS_IWT_H
