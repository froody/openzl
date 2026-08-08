// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_SPHERICAL_RANGE_DECODE_BINDING_H
#define ZSTRONG_TRANSFORMS_SPHERICAL_RANGE_DECODE_BINDING_H

#include "openzl/codecs/spherical_range/graph_spherical_range.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"

ZL_BEGIN_C_DECLS

ZL_Report DI_spherical_range(
        ZL_Decoder* dictx,
        const ZL_Input* ins[]);

#define DI_SPHERICAL_RANGE(id) \
    { .transform_f = DI_spherical_range, .name = "!zl.spherical_range" }

ZL_END_C_DECLS

#endif // ZSTRONG_TRANSFORMS_SPHERICAL_RANGE_DECODE_BINDING_H
