// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZS_ENCODE_TANS4_INTERLEAVED_BINDING_H
#define ZS_ENCODE_TANS4_INTERLEAVED_BINDING_H

#include "openzl/codecs/common/graph_pipe.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

ZL_Report EI_tans4_interleaved(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns);

#define EI_TANS4_INTERLEAVED(id)                        \
    { .gd          = PIPE_GRAPH(id),                    \
      .transform_f = EI_tans4_interleaved,              \
      .name        = "!zl.tans4_interleaved" }

ZL_END_C_DECLS

#endif // ZS_ENCODE_TANS4_INTERLEAVED_BINDING_H
