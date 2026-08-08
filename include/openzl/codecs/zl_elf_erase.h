// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_ELF_ERASE_H
#define ZSTRONG_CODECS_ELF_ERASE_H

#include "openzl/zl_nodes.h"
#include "openzl/zl_opaque_types.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Elf* Adaptive Bit-Erasing Transform for Float Geometry
// Input  : 1 numeric stream (float / double / uint32 / uint64)
// Output : 1 serial stream (compact bitstream)
#define ZL_NODE_ELF_ERASE ZL_MAKE_NODE_ID(ZL_StandardNodeID_elf_erase)

#if defined(__cplusplus)
}
#endif

#endif // ZSTRONG_CODECS_ELF_ERASE_H
