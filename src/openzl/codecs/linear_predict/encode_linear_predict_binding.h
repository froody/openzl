// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_LINEAR_PREDICT_ENCODE_BINDING_H
#define ZSTRONG_CODECS_LINEAR_PREDICT_ENCODE_BINDING_H

#include "openzl/codecs/common/graph_pipe.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

ZL_Report EI_linear_predict(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns);

#define EI_LINEAR_PREDICT(id)                            \
    { .gd          = NUMPIPE_GRAPH(id),                 \
      .transform_f = EI_linear_predict,                 \
      .name        = "!zl.linear_predict" }

ZL_END_C_DECLS

#endif // ZSTRONG_CODECS_LINEAR_PREDICT_ENCODE_BINDING_H
