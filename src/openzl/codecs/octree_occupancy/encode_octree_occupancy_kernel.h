// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_OCTREE_OCCUPANCY_ENCODE_OCTREE_OCCUPANCY_KERNEL_H
#define OPENZL_CODECS_OCTREE_OCCUPANCY_ENCODE_OCTREE_OCCUPANCY_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Returns the maximum buffer size required to store encoded octree occupancy stream.
 */
size_t ZS_octreeOccupancy_encodeBound(size_t srcSize);

/**
 * Encodes a 3D point cloud byte stream into an octree occupancy byte-mask stream.
 *
 * @param dst Output buffer for encoded data.
 * @param dstCapacity Size of output buffer.
 * @param src Input 3D point cloud byte stream.
 * @param srcSize Size of input in bytes.
 * @return Total bytes written to dst, or 0 on error.
 */
size_t ZS_octreeOccupancy_encode(
        void* dst,
        size_t dstCapacity,
        const void* src,
        size_t srcSize);

#if defined(__cplusplus)
}
#endif

#endif // OPENZL_CODECS_OCTREE_OCCUPANCY_ENCODE_OCTREE_OCCUPANCY_KERNEL_H
