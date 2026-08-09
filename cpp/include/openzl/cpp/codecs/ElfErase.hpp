// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_elf_erase.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
class ElfErase : public SimplePipeNode<ElfErase> {
   public:
    static constexpr NodeID node = ZL_NODE_ELF_ERASE;

    static constexpr NodeMetadata<1, 1> metadata = {
        .inputs           = { InputMetadata{ .type = Type::Numeric, .name = "floats" } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Numeric,
                                              .name = "erased" } },
        .description = "Elf* adaptive bit-erasing float transform"
    };
};
} // namespace nodes
} // namespace openzl
