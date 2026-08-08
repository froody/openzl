// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_BITPLANE_SPLIT_ENCODE_BITPLANE_SPLIT_BINDING_H
#define OPENZL_CODECS_BITPLANE_SPLIT_ENCODE_BITPLANE_SPLIT_BINDING_H

#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

ZL_Report EI_bitplane_split(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns);

#define EI_BITPLANE_SPLIT(id)                                                           \
    { .gd          = { .CTid       = id,                                                \
                       .inputTypes = ZL_STREAMTYPELIST(ZL_Type_numeric),                \
                       .soTypes    = ZL_STREAMTYPELIST(ZL_Type_numeric, ZL_Type_numeric) }, \
      .transform_f = EI_bitplane_split,                                                 \
      .name        = "!zl.bitplane_split" }

ZL_END_C_DECLS

#endif // OPENZL_CODECS_BITPLANE_SPLIT_ENCODE_BITPLANE_SPLIT_BINDING_H
