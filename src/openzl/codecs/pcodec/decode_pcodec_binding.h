// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZS_DECODE_PCODEC_BINDING_H
#define ZS_DECODE_PCODEC_BINDING_H

#include "openzl/shared/portability.h"
#include "openzl/zl_data.h"
#include "openzl/zl_dtransform.h"

ZL_BEGIN_C_DECLS

ZL_Report DI_pcodec_int(ZL_Decoder* dictx, const ZL_Input* ins[]);

#define PCODEC_GRAPH(id)                                      \
    { .CTid       = ZL_StandardTransformID_pcodec_int,        \
      .inputTypes = ZL_STREAMTYPELIST(ZL_Type_numeric),       \
      .soTypes    = ZL_STREAMTYPELIST(ZL_Type_numeric, ZL_Type_numeric) }

#define DI_PCODEC_INT(id) { .transform_f = DI_pcodec_int, .name = "pcodec" }

ZL_END_C_DECLS

#endif // ZS_DECODE_PCODEC_BINDING_H
