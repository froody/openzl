// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_CODECS_IWT_ENCODE_IWT_BINDING_H
#define ZSTRONG_CODECS_IWT_ENCODE_IWT_BINDING_H

#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

// ZL_TypedEncoderFn
ZL_Report EI_iwt_int(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns);

#define EI_IWT_INT(id)                                                                  \
    { .gd          = { .CTid       = id,                                               \
                       .inputTypes = ZL_STREAMTYPELIST(ZL_Type_numeric),               \
                       .soTypes    = ZL_STREAMTYPELIST(ZL_Type_numeric, ZL_Type_numeric) }, \
      .transform_f = EI_iwt_int,                                                       \
      .name        = "!zl.iwt_int" }

ZL_END_C_DECLS

#endif // ZSTRONG_CODECS_IWT_ENCODE_IWT_BINDING_H
