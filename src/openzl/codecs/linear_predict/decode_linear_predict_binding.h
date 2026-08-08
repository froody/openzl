// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_LINEAR_PREDICT_DECODE_BINDING_H
#define ZSTRONG_CODECS_LINEAR_PREDICT_DECODE_BINDING_H

#include "openzl/codecs/common/graph_pipe.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_dtransform.h"

ZL_BEGIN_C_DECLS

ZL_Report DI_linear_predict(ZL_Decoder* dictx, const ZL_Input* ins[]);

#define DI_LINEAR_PREDICT(id) { .transform_f = DI_linear_predict, .name = "!zl.linear_predict" }

ZL_END_C_DECLS

#endif // ZSTRONG_CODECS_LINEAR_PREDICT_DECODE_BINDING_H
