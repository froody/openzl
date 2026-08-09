// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_octree_occupancy.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
class OctreeOccupancy : public SimplePipeNode<OctreeOccupancy> {
   public:
    static constexpr NodeID node = ZL_NODE_OCTREE_OCCUPANCY;

    static constexpr NodeMetadata<1, 1> metadata = {
        .inputs           = { InputMetadata{ .type = Type::Struct, .name = "xyz" } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Numeric,
                                              .name = "occupancy" } },
        .description = "Octree occupancy byte-mask encoding transform"
    };
};
} // namespace nodes
} // namespace openzl
