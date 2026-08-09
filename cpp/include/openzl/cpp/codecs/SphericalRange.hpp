// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_spherical_range.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
class SphericalRange : public SimplePipeNode<SphericalRange> {
   public:
    static constexpr NodeID node = ZL_NODE_SPHERICAL_RANGE;

    static constexpr NodeMetadata<1, 1> metadata = {
        .inputs           = { InputMetadata{ .type = Type::Struct, .name = "xyz" } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Numeric,
                                              .name = "spherical" } },
        .description = "Spherical Range image projection transform"
    };
};
} // namespace nodes
} // namespace openzl
