// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZS_DECODE_TANS4_INTERLEAVED_BINDING_H
#define ZS_DECODE_TANS4_INTERLEAVED_BINDING_H

#include "openzl/shared/portability.h"
#include "openzl/zl_dtransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

ZL_Report DI_tans4_interleaved(ZL_Decoder* dictx, const ZL_Input* ins[]);

#define DI_TANS4_INTERLEAVED(id)                                                                     \
    {                                                                                                \
        .transform_f = DI_tans4_interleaved,                                                         \
        .name        = "!zl.tans4_interleaved",                                                      \
    }

ZL_END_C_DECLS

#endif // ZS_DECODE_TANS4_INTERLEAVED_BINDING_H
