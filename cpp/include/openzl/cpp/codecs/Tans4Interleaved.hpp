// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_tans4_interleaved.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
class Tans4Interleaved : public SimplePipeNode<Tans4Interleaved> {
   public:
    static constexpr NodeID node = ZL_NODE_TANS4_INTERLEAVED;

    static constexpr NodeMetadata<1, 1> metadata = {
        .inputs           = { InputMetadata{ .type = Type::Numeric, .name = "symbols" } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Serial,
                                              .name = "tans_substreams" } },
        .description = "4-Way interleaved tANS entropy coder transform"
    };
};
} // namespace nodes
} // namespace openzl
