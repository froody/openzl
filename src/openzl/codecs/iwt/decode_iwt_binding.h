// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_IWT_DECODE_IWT_BINDING_H
#define ZSTRONG_CODECS_IWT_DECODE_IWT_BINDING_H

#include "openzl/shared/portability.h"
#include "openzl/zl_data.h"
#include "openzl/zl_dtransform.h"

ZL_BEGIN_C_DECLS

// ZL_TypedDecoderFn
ZL_Report DI_iwt_int(ZL_Decoder* dictx, const ZL_Input* ins[]);

#define IWT_GRAPH(id)                                         \
    { .CTid       = ZL_StandardTransformID_iwt_int,           \
      .inputTypes = ZL_STREAMTYPELIST(ZL_Type_numeric),       \
      .soTypes    = ZL_STREAMTYPELIST(ZL_Type_numeric, ZL_Type_numeric) }

#define DI_IWT_INT(id) { .transform_f = DI_iwt_int, .name = "iwt" }

ZL_END_C_DECLS

#endif // ZSTRONG_CODECS_IWT_DECODE_IWT_BINDING_H
