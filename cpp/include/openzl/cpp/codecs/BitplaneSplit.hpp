// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_bitplane_split.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
namespace detail {
template <typename NodeT>
class BitplaneSplitNode : public Node {
   public:
    BitplaneSplitNode() = default;

    NodeID baseNode() const override
    {
        return NodeT::node;
    }

    GraphID
    operator()(Compressor& compressor, GraphID msb, GraphID lsb) const
    {
        return buildGraph(
                compressor,
                std::initializer_list<GraphID>{ msb, lsb });
    }

    ~BitplaneSplitNode() override = default;
};
} // namespace detail

class BitplaneSplit : public detail::BitplaneSplitNode<BitplaneSplit> {
   public:
    static constexpr NodeID node = ZL_NODE_BITPLANE_SPLIT;

    static constexpr NodeMetadata<1, 2> metadata = {
        .inputs = { InputMetadata{ .type = Type::Numeric, .name = "input" } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Numeric,
                                              .name = "msb" },
                              OutputMetadata{ .type = Type::Numeric,
                                              .name = "lsb" } },
        .description = "Bit-plane splitting MSB/LSB stream decoupling transform"
    };
};
} // namespace nodes
} // namespace openzl
