// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "custom_parsers/shared_components/clustering.h"
#include "custom_parsers/shared_components/numeric_graphs.h"
#include "custom_parsers/shared_components/string_graphs.h"
#include "openzl/compress/graphs/generic_clustering_graph.h"

#include "openzl/codecs/zl_bitplane_split.h"
#include "openzl/codecs/zl_elf_erase.h"
#include "openzl/codecs/zl_iwt.h"
#include "openzl/codecs/zl_linear_predict.h"
#include "openzl/codecs/zl_morton_3d.h"
#include "openzl/codecs/zl_octree_occupancy.h"
#include "openzl/codecs/zl_pcodec.h"
#include "openzl/codecs/zl_quars.h"
#include "openzl/codecs/zl_spherical_range.h"
#include "openzl/codecs/zl_tans4_interleaved.h"

ZL_GraphID ZS2_createGraph_genericClustering(ZL_Compressor* compressor)
{
    static const size_t NUM_SUCCESSORS          = 19;
    const ZL_GraphID successors[NUM_SUCCESSORS] = {
        ZL_GRAPH_STORE,
        ZL_GRAPH_FIELD_LZ,
        ZL_GRAPH_ZSTD,
        ZL_GRAPH_COMPRESS_GENERIC,
        openzl::custom_parsers::ZL_Compressor_registerRangePack(compressor),
        openzl::custom_parsers::ZL_Compressor_registerRangePackZstd(compressor),
        openzl::custom_parsers::ZL_Compressor_registerTokenizeSorted(
                compressor),
        openzl::custom_parsers::ZL_Compressor_registerDeltaFieldLZ(compressor),
        openzl::custom_parsers::ZL_Compressor_registerStringTokenize(compressor),
        ZL_Compressor_registerStaticGraph_fromNode1o(
                compressor, ZL_NODE_MORTON_3D, ZL_GRAPH_FIELD_LZ),
        ZL_Compressor_registerStaticGraph_fromNode1o(
                compressor, ZL_NODE_SPHERICAL_RANGE, ZL_GRAPH_FIELD_LZ),
        ZL_Compressor_registerStaticGraph_fromNode1o(
                compressor, ZL_NODE_QUARS_INT, ZL_GRAPH_FIELD_LZ),
        ZL_Compressor_registerStaticGraph_fromNode1o(
                compressor, ZL_NODE_ELF_ERASE, ZL_GRAPH_FIELD_LZ),
        ZL_Compressor_registerStaticGraph_fromNode1o(
                compressor, ZL_NODE_BITPLANE_SPLIT, ZL_GRAPH_FIELD_LZ),
        ZL_Compressor_registerStaticGraph_fromNode1o(
                compressor, ZL_NODE_OCTREE_OCCUPANCY, ZL_GRAPH_FIELD_LZ),
        ZL_Compressor_registerStaticGraph_fromNode1o(
                compressor, ZL_NODE_LINEAR_PREDICT, ZL_GRAPH_FIELD_LZ),
        ZL_Compressor_registerStaticGraph_fromNode1o(
                compressor, ZL_NODE_TANS4_INTERLEAVED, ZL_GRAPH_FIELD_LZ),
        ZL_Compressor_registerStaticGraph_fromNode1o(
                compressor, ZL_NODE_IWT_INT, ZL_GRAPH_FIELD_LZ),
        ZL_Compressor_registerStaticGraph_fromNode1o(
                compressor, ZL_NODE_PCODEC_INT, ZL_GRAPH_FIELD_LZ)
    };

    ZL_ClusteringConfig config = {};

    std::array<ZL_NodeID, 4> clusteringCodecs = { ZL_NODE_CONCAT_SERIAL,
                                                  ZL_NODE_CONCAT_STRUCT,
                                                  ZL_NODE_CONCAT_NUMERIC,
                                                  ZL_NODE_CONCAT_STRING };

    return ZL_Clustering_registerGraphWithCustomClusteringCodecs(
            compressor,
            &config,
            successors,
            NUM_SUCCESSORS,
            clusteringCodecs.data(),
            clusteringCodecs.size());
}
