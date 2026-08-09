// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_quars.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
class QuaRsInt : public SimplePipeNode<QuaRsInt> {
   public:
    static constexpr NodeID node = ZL_NODE_QUARS_INT;

    static constexpr NodeMetadata<1, 1> metadata = {
        .inputs           = { InputMetadata{ .type = Type::Numeric, .name = "integers" } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Numeric,
                                              .name = "reshaped" } },
        .description = "QuaRs integer distribution reshaping transform"
    };
};
} // namespace nodes
} // namespace openzl
