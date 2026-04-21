// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_cat_pack.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
class CatPack : public SimplePipeNode<CatPack> {
   public:
    static inline constexpr NodeID node = ZL_NODE_CAT_PACK;

    static constexpr NodeMetadata<1, 1> metadata = {
        .inputs           = { InputMetadata{ .type = Type::Numeric } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Numeric } },
        .description =
                "Finds unique categorical values and bitpacks them into 1, 2, or 4 bits.",
    };
};
} // namespace nodes
} // namespace openzl
