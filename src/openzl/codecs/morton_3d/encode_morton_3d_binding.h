// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_MORTON_3D_ENCODE_MORTON_3D_BINDING_H
#define OPENZL_CODECS_MORTON_3D_ENCODE_MORTON_3D_BINDING_H

#include "openzl/common/wire_format.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

// ZL_TypedEncoderFn
ZL_Report EI_morton_3d(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns);

#define MORTON_3D_GRAPH(id)                                                                     \
    { .CTid       = ZL_StandardTransformID_morton_3d,                                          \
      .inputTypes = ZL_STREAMTYPELIST(ZL_Type_numeric, ZL_Type_numeric, ZL_Type_numeric), \
      .soTypes    = ZL_STREAMTYPELIST(ZL_Type_numeric) }

#define EI_MORTON_3D(id)                  \
    { .gd          = MORTON_3D_GRAPH(id), \
      .transform_f = EI_morton_3d,        \
      .name        = "!zl.morton_3d" }

ZL_END_C_DECLS

#endif // OPENZL_CODECS_MORTON_3D_ENCODE_MORTON_3D_BINDING_H
