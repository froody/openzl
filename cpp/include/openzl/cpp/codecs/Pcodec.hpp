// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "openzl/codecs/zl_pcodec.h"
#include "openzl/cpp/Compressor.hpp"
#include "openzl/cpp/codecs/Metadata.hpp"
#include "openzl/cpp/codecs/Node.hpp"

namespace openzl {
namespace nodes {
namespace detail {
template <typename NodeT>
class PcodecNode : public Node {
   public:
    PcodecNode() = default;

    NodeID baseNode() const override
    {
        return NodeT::node;
    }

    GraphID
    operator()(Compressor& compressor, GraphID latents, GraphID residuals) const
    {
        return buildGraph(
                compressor,
                std::initializer_list<GraphID>{ latents, residuals });
    }

    ~PcodecNode() override = default;
};
} // namespace detail

class PcodecInt : public detail::PcodecNode<PcodecInt> {
   public:
    static constexpr NodeID node = ZL_NODE_PCODEC_INT;

    static constexpr NodeMetadata<1, 2> metadata = {
        .inputs = { InputMetadata{ .type = Type::Numeric, .name = "input" } },
        .singletonOutputs = { OutputMetadata{ .type = Type::Numeric,
                                              .name = "latents" },
                              OutputMetadata{ .type = Type::Numeric,
                                              .name = "residuals" } },
        .description = "Pcodec Numeric Sequence Pre-Conditioner (Decomposes into Binned Latents + Offsets)"
    };
};
} // namespace nodes
} // namespace openzl
