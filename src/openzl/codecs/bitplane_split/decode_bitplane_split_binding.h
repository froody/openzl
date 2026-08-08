// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_BITPLANE_SPLIT_DECODE_BITPLANE_SPLIT_BINDING_H
#define OPENZL_CODECS_BITPLANE_SPLIT_DECODE_BITPLANE_SPLIT_BINDING_H

#include "openzl/shared/portability.h"
#include "openzl/zl_data.h"
#include "openzl/zl_dtransform.h"

ZL_BEGIN_C_DECLS

ZL_Report DI_bitplane_split(ZL_Decoder* dictx, const ZL_Input* ins[]);

#define BITPLANE_SPLIT_GRAPH(id)                               \
    { .CTid       = ZL_StandardTransformID_bitplane_split,    \
      .inputTypes = ZL_STREAMTYPELIST(ZL_Type_numeric),       \
      .soTypes    = ZL_STREAMTYPELIST(ZL_Type_numeric, ZL_Type_numeric) }

#define DI_BITPLANE_SPLIT(id) { .transform_f = DI_bitplane_split, .name = "bitplane_split" }

ZL_END_C_DECLS

#endif // OPENZL_CODECS_BITPLANE_SPLIT_DECODE_BITPLANE_SPLIT_BINDING_H
