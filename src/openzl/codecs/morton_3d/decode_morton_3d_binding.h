// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_MORTON_3D_DECODE_MORTON_3D_BINDING_H
#define OPENZL_CODECS_MORTON_3D_DECODE_MORTON_3D_BINDING_H

#include "openzl/common/wire_format.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_data.h"
#include "openzl/zl_dtransform.h"

ZL_BEGIN_C_DECLS

// ZL_TypedDecoderFn
ZL_Report DI_morton_3d(ZL_Decoder* dictx, const ZL_Input* ins[]);

#define MORTON_3D_DECODE_GRAPH(id)                                                             \
    { .CTid       = ZL_StandardTransformID_morton_3d,                                          \
      .inputTypes = ZL_STREAMTYPELIST(ZL_Type_numeric),                                       \
      .soTypes    = ZL_STREAMTYPELIST(ZL_Type_numeric, ZL_Type_numeric, ZL_Type_numeric) }

#define DI_MORTON_3D(id) { .transform_f = DI_morton_3d, .name = "morton_3d" }

ZL_END_C_DECLS

#endif // OPENZL_CODECS_MORTON_3D_DECODE_MORTON_3D_BINDING_H
