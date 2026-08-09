// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_linear_predict.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
class LinearPredict : public SimplePipeNode<LinearPredict> {
   public:
    static constexpr NodeID node = ZL_NODE_LINEAR_PREDICT;

    static constexpr NodeMetadata<1, 1> metadata = {
        .inputs           = { InputMetadata{ .type = Type::Numeric, .name = "input" } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Numeric,
                                              .name = "residuals" } },
        .description = "Inter-ring and intra-neighbor linear residual predictor transform"
    };
};
} // namespace nodes
} // namespace openzl
