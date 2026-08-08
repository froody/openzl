// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/octree_occupancy/decode_octree_occupancy_kernel.h"

#include <stdlib.h>
#include <string.h>

#include "openzl/shared/mem.h"
#include "openzl/shared/varint.h"

#define OCTREE_MAGIC 0x4F435452 /* "OCTR" */

typedef struct {
    uint64_t x, y, z;
} LeafVoxel;

typedef struct {
    uint64_t x, y, z;
} DecNode;

size_t ZS_octreeOccupancy_getDecodedSize(const void* src, size_t srcSize)
{
    if (!src || srcSize < 6) return 0;

    const uint8_t* ptr = (const uint8_t*)src;
    const uint8_t* end = ptr + srcSize;

    uint32_t magic = ZL_readLE32(ptr); ptr += 4;
    if (magic != OCTREE_MAGIC) return 0;

    uint8_t component_width = *ptr++;
    if (component_width != 1 && component_width != 2 &&
        component_width != 4 && component_width != 8) {
        return 0;
    }

    uint8_t flags = *ptr++; (void)flags;

    ZL_RESULT_OF(uint64_t) resN = ZL_varintDecode(&ptr, end);
    if (ZL_RES_isError(resN)) return 0;
    uint64_t N = ZL_RES_value(resN);

    ZL_RESULT_OF(uint64_t) resR = ZL_varintDecode(&ptr, end);
    if (ZL_RES_isError(resR)) return 0;
    uint64_t R = ZL_RES_value(resR);

    if (N > 1000000000ULL || R > 1000000000ULL) return 0;

    return (size_t)N * 3 * (size_t)component_width + (size_t)R;
}

size_t ZS_octreeOccupancy_decode(
        void* dst,
        size_t dstCapacity,
        const void* src,
        size_t srcSize)
{
    if (!dst || !src || srcSize < 6) return 0;

    const uint8_t* ptr = (const uint8_t*)src;
    const uint8_t* end = ptr + srcSize;

    uint32_t magic = ZL_readLE32(ptr); ptr += 4;
    if (magic != OCTREE_MAGIC) return 0;

    uint8_t component_width = *ptr++;
    if (component_width != 1 && component_width != 2 &&
        component_width != 4 && component_width != 8) {
        return 0;
    }

    uint8_t flags = *ptr++;
    uint8_t is_identity_order = flags & 1;

    ZL_RESULT_OF(uint64_t) resN = ZL_varintDecode(&ptr, end);
    if (ZL_RES_isError(resN)) return 0;
    size_t N = (size_t)ZL_RES_value(resN);

    ZL_RESULT_OF(uint64_t) resR = ZL_varintDecode(&ptr, end);
    if (ZL_RES_isError(resR)) return 0;
    size_t R = (size_t)ZL_RES_value(resR);

    size_t point_bytes = 3 * (size_t)component_width;
    size_t total_expected = N * point_bytes + R;
    if (dstCapacity < total_expected) return 0;

    uint8_t* out_bytes = (uint8_t*)dst;

    if (N == 0) {
        if ((size_t)(end - ptr) < R) return 0;
        if (R > 0) {
            memcpy(out_bytes, ptr, R);
        }
        return R;
    }

    if ((size_t)(end - ptr) < point_bytes + 1) return 0;

    uint64_t X_min = ZL_readN(ptr, component_width); ptr += component_width;
    uint64_t Y_min = ZL_readN(ptr, component_width); ptr += component_width;
    uint64_t Z_min = ZL_readN(ptr, component_width); ptr += component_width;

    uint8_t depth = *ptr++;
    if (depth > 64) return 0;

    ZL_RESULT_OF(uint64_t) resM = ZL_varintDecode(&ptr, end);
    if (ZL_RES_isError(resM)) return 0;
    size_t num_masks = (size_t)ZL_RES_value(resM);

    if ((size_t)(end - ptr) < num_masks) return 0;
    const uint8_t* occupancy_masks = ptr;
    ptr += num_masks;

    LeafVoxel* leaf_voxels = NULL;
    size_t leaf_count = 0;

    if (depth > 0) {
        DecNode* current_level = (DecNode*)malloc(sizeof(DecNode));
        if (!current_level) return 0;
        size_t current_count = 1;
        current_level[0].x = 0;
        current_level[0].y = 0;
        current_level[0].z = 0;

        size_t mask_idx = 0;

        for (uint8_t L = 0; L < depth; ++L) {
            uint64_t half = (uint64_t)1 << (depth - L - 1);
            size_t next_capacity = 0;

            for (size_t i = 0; i < current_count; ++i) {
                if (mask_idx >= num_masks) {
                    free(current_level);
                    free(leaf_voxels);
                    return 0;
                }
                uint8_t mask = occupancy_masks[mask_idx++];
                for (int c = 0; c < 8; ++c) {
                    if (mask & (1 << c)) {
                        next_capacity++;
                    }
                }
            }

            DecNode* next_level = (DecNode*)malloc(next_capacity * sizeof(DecNode));
            if (!next_level) {
                free(current_level);
                free(leaf_voxels);
                return 0;
            }

            size_t rewind_mask_idx = mask_idx - current_count;
            size_t next_idx = 0;

            for (size_t i = 0; i < current_count; ++i) {
                DecNode parent = current_level[i];
                uint8_t mask = occupancy_masks[rewind_mask_idx++];
                for (int c = 0; c < 8; ++c) {
                    if (mask & (1 << c)) {
                        next_level[next_idx].x = parent.x + ((c & 1) ? half : 0);
                        next_level[next_idx].y = parent.y + ((c & 2) ? half : 0);
                        next_level[next_idx].z = parent.z + ((c & 4) ? half : 0);
                        next_idx++;
                    }
                }
            }

            free(current_level);
            current_level = next_level;
            current_count = next_capacity;

            if (L + 1 == depth) {
                leaf_count = current_count;
                leaf_voxels = (LeafVoxel*)malloc(leaf_count * sizeof(LeafVoxel));
                if (!leaf_voxels) {
                    free(current_level);
                    return 0;
                }
                for (size_t i = 0; i < leaf_count; ++i) {
                    leaf_voxels[i].x = current_level[i].x;
                    leaf_voxels[i].y = current_level[i].y;
                    leaf_voxels[i].z = current_level[i].z;
                }
                free(current_level);
                current_level = NULL;
                current_count = 0;
            }
        }
    } else {
        leaf_count = 1;
        leaf_voxels = (LeafVoxel*)malloc(sizeof(LeafVoxel));
        if (!leaf_voxels) return 0;
        leaf_voxels[0].x = 0;
        leaf_voxels[0].y = 0;
        leaf_voxels[0].z = 0;
    }

    if (is_identity_order) {
        if (leaf_count != N) {
            free(leaf_voxels);
            return 0;
        }
        for (size_t i = 0; i < N; ++i) {
            uint64_t X = X_min + leaf_voxels[i].x;
            uint64_t Y = Y_min + leaf_voxels[i].y;
            uint64_t Z = Z_min + leaf_voxels[i].z;

            uint8_t* p = out_bytes + i * point_bytes;
            ZL_writeN(p, X, component_width);
            ZL_writeN(p + component_width, Y, component_width);
            ZL_writeN(p + 2 * component_width, Z, component_width);
        }
    } else {
        if (ptr == end) {
            free(leaf_voxels);
            return 0;
        }
        uint8_t index_element_bytes = *ptr++;
        if (index_element_bytes != 1 && index_element_bytes != 2 && index_element_bytes != 4) {
            free(leaf_voxels);
            return 0;
        }
        if ((size_t)(end - ptr) < N * (size_t)index_element_bytes) {
            free(leaf_voxels);
            return 0;
        }

        for (size_t i = 0; i < N; ++i) {
            uint64_t leaf_idx = ZL_readN(ptr, index_element_bytes);
            ptr += index_element_bytes;

            if (leaf_idx >= leaf_count) {
                free(leaf_voxels);
                return 0;
            }

            uint64_t X = X_min + leaf_voxels[leaf_idx].x;
            uint64_t Y = Y_min + leaf_voxels[leaf_idx].y;
            uint64_t Z = Z_min + leaf_voxels[leaf_idx].z;

            uint8_t* p = out_bytes + i * point_bytes;
            ZL_writeN(p, X, component_width);
            ZL_writeN(p + component_width, Y, component_width);
            ZL_writeN(p + 2 * component_width, Z, component_width);
        }
    }

    free(leaf_voxels);

    if ((size_t)(end - ptr) < R) return 0;
    if (R > 0) {
        memcpy(out_bytes + N * point_bytes, ptr, R);
    }

    return total_expected;
}
