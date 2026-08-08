// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_BITPLANE_SPLIT_H
#define ZSTRONG_CODECS_BITPLANE_SPLIT_H

#include "openzl/zl_nodes.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Bit-Plane Splitting & MSB/LSB Stream Decoupling Transform
// Input  : 1 numeric stream (all widths supported: 1, 2, 4, 8 bytes)
// Output : 2 numeric streams (MSB and LSB streams)
//          Output 0: MSB stream
//          Output 1: LSB stream
#define ZL_NODE_BITPLANE_SPLIT ZL_MAKE_NODE_ID(ZL_StandardNodeID_bitplane_split)

#if defined(__cplusplus)
}
#endif

#endif // ZSTRONG_CODECS_BITPLANE_SPLIT_H
