// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_TRANSFORMS_ELF_ERASE_ENCODE_BINDING_H
#define ZSTRONG_TRANSFORMS_ELF_ERASE_ENCODE_BINDING_H

#include "openzl/codecs/conversion/graph_conversion.h"
#include "openzl/shared/portability.h"
#include "openzl/zl_ctransform.h"
#include "openzl/zl_data.h"

ZL_BEGIN_C_DECLS

ZL_Report EI_elf_erase(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns);

#define EI_ELF_ERASE(id)                           \
    { .gd          = CONVERT_NUM_SERIAL_GRAPH(id), \
      .transform_f = EI_elf_erase,                 \
      .name        = "!zl.elf_erase" }

ZL_END_C_DECLS

#endif // ZSTRONG_TRANSFORMS_ELF_ERASE_ENCODE_BINDING_H
