// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZS_ENCODE_PCODEC_BINDING_H
#define ZS_ENCODE_PCODEC_BINDING_H

#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

ZL_Report EI_pcodec_int(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns);

#define EI_PCODEC_INT(id)                                                               \
    { .gd          = { .CTid       = id,                                                \
                       .inputTypes = ZL_STREAMTYPELIST(ZL_Type_numeric),                \
                       .soTypes    = ZL_STREAMTYPELIST(ZL_Type_numeric, ZL_Type_numeric) }, \
      .transform_f = EI_pcodec_int,                                                     \
      .name        = "!zl.pcodec_int" }

ZL_END_C_DECLS

#endif // ZS_ENCODE_PCODEC_BINDING_H
