// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_morton_3d.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
class Morton3D : public SimplePipeNode<Morton3D> {
   public:
    static constexpr NodeID node = ZL_NODE_MORTON_3D;

    static constexpr NodeMetadata<1, 1> metadata = {
        .inputs           = { InputMetadata{ .type = Type::Struct, .name = "xyz" } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Numeric,
                                              .name = "morton" } },
        .description = "3D Morton Z-order curve spatial reordering transform"
    };
};
} // namespace nodes
} // namespace openzl
