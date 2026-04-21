// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_CAT_PACK_ENCODE_CAT_PACK_BINDING_H
#define ZSTRONG_TRANSFORMS_CAT_PACK_ENCODE_CAT_PACK_BINDING_H

#include "openzl/codecs/cat_pack/graph_cat_pack.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h" // ZL_Encoder

ZL_BEGIN_C_DECLS

ZL_Report EI_catPack(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns);

#define EI_CAT_PACK(id)                  \
    { .gd          = CAT_PACK_GRAPH(id), \
      .transform_f = EI_catPack,         \
      .name        = "!zl.cat_pack" }

ZL_END_C_DECLS

#endif
