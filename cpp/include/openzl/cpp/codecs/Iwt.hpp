// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_iwt.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
namespace detail {
template <typename NodeT>
class IwtNode : public Node {
   public:
    IwtNode() = default;

    NodeID baseNode() const override
    {
        return NodeT::node;
    }

    GraphID
    operator()(Compressor& compressor, GraphID approx, GraphID detail) const
    {
        return buildGraph(
                compressor,
                std::initializer_list<GraphID>{ approx, detail });
    }

    ~IwtNode() override = default;
};
} // namespace detail

class IwtInt : public detail::IwtNode<IwtInt> {
   public:
    static constexpr NodeID node = ZL_NODE_IWT_INT;

    static constexpr NodeMetadata<1, 2> metadata = {
        .inputs = { InputMetadata{ .type = Type::Numeric, .name = "input" } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Numeric,
                                              .name = "approx" },
                              OutputMetadata{ .type = Type::Numeric,
                                              .name = "detail" } },
        .description = "1D Integer Wavelet Transform (5/3 LeGall Lifting Scheme)"
    };
};
} // namespace nodes
} // namespace openzl
