// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_ELF_ERASE_DECODE_BINDING_H
#define ZSTRONG_TRANSFORMS_ELF_ERASE_DECODE_BINDING_H

#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

ZL_Report DI_elf_erase(ZL_Decoder* dictx, const ZL_Input* ins[]);

#define ELF_ERASE_TYPED_DECODE_GRAPH(id)                           \
    {                                                              \
        .CTid           = id,                                      \
        .inStreamType   = ZL_Type_serial,                          \
        .outStreamTypes = ZL_STREAMTYPELIST(ZL_Type_numeric),     \
    }

#define ELF_ERASE_GRAPH(id)                                       \
    {                                                             \
        .CTid       = id,                                         \
        .inputTypes = ZL_STREAMTYPELIST(ZL_Type_serial),          \
        .soTypes    = ZL_STREAMTYPELIST(ZL_Type_numeric),         \
    }

#define DI_ELF_ERASE(id)                           \
    { .gd          = ELF_ERASE_TYPED_DECODE_GRAPH(id), \
      .transform_f = DI_elf_erase,                 \
      .name        = "!zl.elf_erase" }

ZL_END_C_DECLS

#endif // ZSTRONG_TRANSFORMS_ELF_ERASE_DECODE_BINDING_H
