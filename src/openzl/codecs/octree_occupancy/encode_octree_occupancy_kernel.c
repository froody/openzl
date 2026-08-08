// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/octree_occupancy/encode_octree_occupancy_kernel.h"

#include <stdlib.h>
#include <string.h>

#include "openzl/shared/mem.h"
#include "openzl/shared/varint.h"

#define OCTREE_MAGIC 0x4F435452 /* "OCTR" */

typedef struct {
    uint64_t x, y, z;
    size_t point_start;
    size_t point_count;
} OctreeNode;

size_t ZS_octreeOccupancy_encodeBound(size_t srcSize)
{
    return srcSize * 4 + 1024;
}

static uint8_t selectComponentWidth(size_t srcSize)
{
    if (srcSize >= 12 && srcSize % 12 == 0) return 4;
    if (srcSize >= 6 && srcSize % 6 == 0) return 2;
    if (srcSize >= 3 && srcSize % 3 == 0) return 1;
    if (srcSize >= 24 && srcSize % 24 == 0) return 8;
    if (srcSize >= 12) return 4;
    if (srcSize >= 6) return 2;
    if (srcSize >= 3) return 1;
    return 4;
}

static uint64_t readComponent(const uint8_t* ptr, uint8_t width)
{
    switch (width) {
        case 1: return ZL_read8(ptr);
        case 2: return ZL_readLE16(ptr);
        case 4: return ZL_readLE32(ptr);
        case 8: return ZL_readLE64(ptr);
        default: return 0;
    }
}

size_t ZS_octreeOccupancy_encode(
        void* dst,
        size_t dstCapacity,
        const void* src,
        size_t srcSize)
{
    if (!dst || (!src && srcSize > 0)) return 0;

    uint8_t component_width = selectComponentWidth(srcSize);
    size_t point_bytes = 3 * (size_t)component_width;
    size_t N = srcSize / point_bytes;
    size_t R = srcSize % point_bytes;

    const uint8_t* src_bytes = (const uint8_t*)src;

    uint64_t X_min = 0, X_max = 0;
    uint64_t Y_min = 0, Y_max = 0;
    uint64_t Z_min = 0, Z_max = 0;

    uint64_t* x_rel = NULL;
    uint64_t* y_rel = NULL;
    uint64_t* z_rel = NULL;

    if (N > 0) {
        x_rel = (uint64_t*)malloc(N * sizeof(uint64_t));
        y_rel = (uint64_t*)malloc(N * sizeof(uint64_t));
        z_rel = (uint64_t*)malloc(N * sizeof(uint64_t));
        if (!x_rel || !y_rel || !z_rel) {
            free(x_rel); free(y_rel); free(z_rel);
            return 0;
        }

        X_min = readComponent(src_bytes, component_width);
        X_max = X_min;
        Y_min = readComponent(src_bytes + component_width, component_width);
        Y_max = Y_min;
        Z_min = readComponent(src_bytes + 2 * component_width, component_width);
        Z_max = Z_min;

        for (size_t i = 1; i < N; ++i) {
            const uint8_t* p = src_bytes + i * point_bytes;
            uint64_t x = readComponent(p, component_width);
            uint64_t y = readComponent(p + component_width, component_width);
            uint64_t z = readComponent(p + 2 * component_width, component_width);

            if (x < X_min) X_min = x;
            if (x > X_max) X_max = x;
            if (y < Y_min) Y_min = y;
            if (y > Y_max) Y_max = y;
            if (z < Z_min) Z_min = z;
            if (z > Z_max) Z_max = z;
        }

        for (size_t i = 0; i < N; ++i) {
            const uint8_t* p = src_bytes + i * point_bytes;
            uint64_t x = readComponent(p, component_width);
            uint64_t y = readComponent(p + component_width, component_width);
            uint64_t z = readComponent(p + 2 * component_width, component_width);

            x_rel[i] = x - X_min;
            y_rel[i] = y - Y_min;
            z_rel[i] = z - Z_min;
        }
    }

    uint64_t M_x = X_max - X_min;
    uint64_t M_y = Y_max - Y_min;
    uint64_t M_z = Z_max - Z_min;
    uint64_t M_max = M_x;
    if (M_y > M_max) M_max = M_y;
    if (M_z > M_max) M_max = M_z;

    uint8_t depth = 0;
    if (M_max > 0) {
        while (depth < 64 && ((uint64_t)1 << depth) <= M_max) {
            depth++;
        }
    }

    size_t occupancy_capacity = N * 8 + 256;
    uint8_t* occupancy_masks = (uint8_t*)malloc(occupancy_capacity);
    size_t num_masks = 0;

    size_t* point_order = NULL;
    size_t* temp_order = NULL;
    size_t* leaf_of_point = NULL;

    if (N > 0) {
        point_order = (size_t*)malloc(N * sizeof(size_t));
        temp_order = (size_t*)malloc(N * sizeof(size_t));
        leaf_of_point = (size_t*)malloc(N * sizeof(size_t));
        if (!occupancy_masks || !point_order || !temp_order || !leaf_of_point) {
            free(x_rel); free(y_rel); free(z_rel);
            free(occupancy_masks); free(point_order); free(temp_order); free(leaf_of_point);
            return 0;
        }
        for (size_t i = 0; i < N; ++i) point_order[i] = i;
    }

    size_t leaf_count = 0;

    if (N > 0 && depth > 0) {
        OctreeNode* current_level = (OctreeNode*)malloc(sizeof(OctreeNode));
        size_t current_count = 1;
        current_level[0].x = 0;
        current_level[0].y = 0;
        current_level[0].z = 0;
        current_level[0].point_start = 0;
        current_level[0].point_count = N;

        for (uint8_t L = 0; L < depth; ++L) {
            uint64_t half = (uint64_t)1 << (depth - L - 1);

            size_t next_capacity = 0;
            OctreeNode* next_level = NULL;

            for (size_t node_i = 0; node_i < current_count; ++node_i) {
                OctreeNode parent = current_level[node_i];
                uint8_t mask = 0;
                size_t counts[8] = {0};

                for (size_t j = 0; j < parent.point_count; ++j) {
                    size_t p_idx = point_order[parent.point_start + j];
                    uint8_t cx = (x_rel[p_idx] >= parent.x + half) ? 1 : 0;
                    uint8_t cy = (y_rel[p_idx] >= parent.y + half) ? 2 : 0;
                    uint8_t cz = (z_rel[p_idx] >= parent.z + half) ? 4 : 0;
                    uint8_t child_idx = cx | cy | cz;
                    mask |= (uint8_t)(1 << child_idx);
                    counts[child_idx]++;
                }

                if (num_masks >= occupancy_capacity) {
                    occupancy_capacity *= 2;
                    uint8_t* new_masks = (uint8_t*)realloc(occupancy_masks, occupancy_capacity);
                    if (!new_masks) {
                        free(current_level);
                        free(x_rel); free(y_rel); free(z_rel);
                        free(occupancy_masks); free(point_order); free(temp_order); free(leaf_of_point);
                        return 0;
                    }
                    occupancy_masks = new_masks;
                }
                occupancy_masks[num_masks++] = mask;

                size_t offsets[8];
                offsets[0] = parent.point_start;
                for (int c = 1; c < 8; ++c) {
                    offsets[c] = offsets[c - 1] + counts[c - 1];
                }

                memcpy(temp_order, point_order + parent.point_start, parent.point_count * sizeof(size_t));
                for (size_t j = 0; j < parent.point_count; ++j) {
                    size_t p_idx = temp_order[j];
                    uint8_t cx = (x_rel[p_idx] >= parent.x + half) ? 1 : 0;
                    uint8_t cy = (y_rel[p_idx] >= parent.y + half) ? 2 : 0;
                    uint8_t cz = (z_rel[p_idx] >= parent.z + half) ? 4 : 0;
                    uint8_t child_idx = cx | cy | cz;
                    point_order[offsets[child_idx]++] = p_idx;
                }

                if (L + 1 < depth) {
                    for (int c = 0; c < 8; ++c) {
                        if (counts[c] > 0) {
                            next_capacity++;
                        }
                    }
                } else {
                    for (int c = 0; c < 8; ++c) {
                        if (counts[c] > 0) {
                            size_t leaf_idx = leaf_count++;
                            size_t start = offsets[c] - counts[c];
                            for (size_t j = 0; j < counts[c]; ++j) {
                                size_t p_idx = point_order[start + j];
                                leaf_of_point[p_idx] = leaf_idx;
                            }
                        }
                    }
                }
            }

            if (L + 1 < depth) {
                next_level = (OctreeNode*)malloc(next_capacity * sizeof(OctreeNode));
                if (!next_level) {
                    free(current_level);
                    free(x_rel); free(y_rel); free(z_rel);
                    free(occupancy_masks); free(point_order); free(temp_order); free(leaf_of_point);
                    return 0;
                }
                size_t next_idx = 0;
                for (size_t node_i = 0; node_i < current_count; ++node_i) {
                    OctreeNode parent = current_level[node_i];
                    size_t counts[8] = {0};
                    for (size_t j = 0; j < parent.point_count; ++j) {
                        size_t p_idx = point_order[parent.point_start + j];
                        uint8_t cx = (x_rel[p_idx] >= parent.x + half) ? 1 : 0;
                        uint8_t cy = (y_rel[p_idx] >= parent.y + half) ? 2 : 0;
                        uint8_t cz = (z_rel[p_idx] >= parent.z + half) ? 4 : 0;
                        uint8_t child_idx = cx | cy | cz;
                        counts[child_idx]++;
                    }
                    size_t cur_start = parent.point_start;
                    for (int c = 0; c < 8; ++c) {
                        if (counts[c] > 0) {
                            next_level[next_idx].x = parent.x + ((c & 1) ? half : 0);
                            next_level[next_idx].y = parent.y + ((c & 2) ? half : 0);
                            next_level[next_idx].z = parent.z + ((c & 4) ? half : 0);
                            next_level[next_idx].point_start = cur_start;
                            next_level[next_idx].point_count = counts[c];
                            cur_start += counts[c];
                            next_idx++;
                        }
                    }
                }
                free(current_level);
                current_level = next_level;
                current_count = next_capacity;
            } else {
                free(current_level);
                current_level = NULL;
                current_count = 0;
            }
        }
    } else if (N > 0 && depth == 0) {
        leaf_count = 1;
        for (size_t i = 0; i < N; ++i) {
            leaf_of_point[i] = 0;
        }
    }

    uint8_t is_identity_order = 1;
    if (N > 0) {
        for (size_t i = 0; i < N; ++i) {
            if (leaf_of_point[i] != i) {
                is_identity_order = 0;
                break;
            }
        }
    }

    uint8_t index_element_bytes = 1;
    if (leaf_count > 65536) {
        index_element_bytes = 4;
    } else if (leaf_count > 256) {
        index_element_bytes = 2;
    }

    uint8_t* out_ptr = (uint8_t*)dst;
    uint8_t* out_end = out_ptr + dstCapacity;

    if ((size_t)(out_end - out_ptr) < 6) {
        free(x_rel); free(y_rel); free(z_rel);
        free(occupancy_masks); free(point_order); free(temp_order); free(leaf_of_point);
        return 0;
    }

    ZL_writeLE32(out_ptr, OCTREE_MAGIC); out_ptr += 4;
    *out_ptr++ = component_width;
    *out_ptr++ = is_identity_order;

    out_ptr += ZL_varintEncode((uint64_t)N, out_ptr);
    out_ptr += ZL_varintEncode((uint64_t)R, out_ptr);

    if (N > 0) {
        size_t bbox_bytes = 3 * (size_t)component_width;
        if ((size_t)(out_end - out_ptr) < bbox_bytes + 1) {
            free(x_rel); free(y_rel); free(z_rel);
            free(occupancy_masks); free(point_order); free(temp_order); free(leaf_of_point);
            return 0;
        }
        ZL_writeN(out_ptr, X_min, component_width); out_ptr += component_width;
        ZL_writeN(out_ptr, Y_min, component_width); out_ptr += component_width;
        ZL_writeN(out_ptr, Z_min, component_width); out_ptr += component_width;
        *out_ptr++ = depth;

        out_ptr += ZL_varintEncode((uint64_t)num_masks, out_ptr);

        if ((size_t)(out_end - out_ptr) < num_masks) {
            free(x_rel); free(y_rel); free(z_rel);
            free(occupancy_masks); free(point_order); free(temp_order); free(leaf_of_point);
            return 0;
        }
        memcpy(out_ptr, occupancy_masks, num_masks);
        out_ptr += num_masks;

        if (!is_identity_order) {
            if ((size_t)(out_end - out_ptr) < 1 + N * index_element_bytes) {
                free(x_rel); free(y_rel); free(z_rel);
                free(occupancy_masks); free(point_order); free(temp_order); free(leaf_of_point);
                return 0;
            }
            *out_ptr++ = index_element_bytes;
            for (size_t i = 0; i < N; ++i) {
                uint64_t leaf_idx = (uint64_t)leaf_of_point[i];
                ZL_writeN(out_ptr, leaf_idx, index_element_bytes);
                out_ptr += index_element_bytes;
            }
        }
    }

    if (R > 0) {
        if ((size_t)(out_end - out_ptr) < R) {
            free(x_rel); free(y_rel); free(z_rel);
            free(occupancy_masks); free(point_order); free(temp_order); free(leaf_of_point);
            return 0;
        }
        memcpy(out_ptr, src_bytes + N * point_bytes, R);
        out_ptr += R;
    }

    free(x_rel); free(y_rel); free(z_rel);
    free(occupancy_masks); free(point_order); free(temp_order); free(leaf_of_point);

    return (size_t)(out_ptr - (uint8_t*)dst);
}
