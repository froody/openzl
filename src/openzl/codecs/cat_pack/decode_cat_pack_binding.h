// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_CAT_PACK_DECODE_CAT_PACK_BINDING_H
#define ZSTRONG_TRANSFORMS_CAT_PACK_DECODE_CAT_PACK_BINDING_H

#include "openzl/codecs/cat_pack/graph_cat_pack.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_dtransform.h" // ZL_Decoder

ZL_BEGIN_C_DECLS

ZL_Report DI_catPack(ZL_Decoder* dictx, const ZL_Input* ins[]);

#define DI_CAT_PACK(id) { .transform_f = DI_catPack, .name = "cat pack" }

ZL_END_C_DECLS

#endif
