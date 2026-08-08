// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_SPHERICAL_RANGE_ENCODE_BINDING_H
#define ZSTRONG_TRANSFORMS_SPHERICAL_RANGE_ENCODE_BINDING_H

#include "openzl/codecs/spherical_range/graph_spherical_range.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"

ZL_BEGIN_C_DECLS

ZL_Report EI_spherical_range(
        ZL_Encoder* eictx,
        const ZL_Input* ins[],
        size_t nbIns);

#define EI_SPHERICAL_RANGE(id)                      \
    {                                                \
        .gd          = SPHERICAL_RANGE_GRAPH(id),    \
        .transform_f = EI_spherical_range,          \
        .name        = "!zl.spherical_range"         \
    }

ZL_END_C_DECLS

#endif // ZSTRONG_TRANSFORMS_SPHERICAL_RANGE_ENCODE_BINDING_H
