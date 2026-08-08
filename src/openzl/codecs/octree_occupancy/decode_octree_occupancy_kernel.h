// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef OPENZL_CODECS_OCTREE_OCCUPANCY_DECODE_OCTREE_OCCUPANCY_KERNEL_H
#define OPENZL_CODECS_OCTREE_OCCUPANCY_DECODE_OCTREE_OCCUPANCY_KERNEL_H

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Reads the compressed octree occupancy stream and returns the exact decoded output byte size.
 * Returns 0 on error / invalid stream.
 */
size_t ZS_octreeOccupancy_getDecodedSize(const void* src, size_t srcSize);

/**
 * Decodes an octree occupancy byte-mask stream back into the original 3D point cloud byte stream.
 *
 * @param dst Output buffer for decoded 3D point cloud.
 * @param dstCapacity Size of output buffer.
 * @param src Input octree occupancy encoded stream.
 * @param srcSize Size of encoded stream.
 * @return Total bytes written to dst, or 0 on error.
 */
size_t ZS_octreeOccupancy_decode(
        void* dst,
        size_t dstCapacity,
        const void* src,
        size_t srcSize);

#if defined(__cplusplus)
}
#endif

#endif // OPENZL_CODECS_OCTREE_OCCUPANCY_DECODE_OCTREE_OCCUPANCY_KERNEL_H
