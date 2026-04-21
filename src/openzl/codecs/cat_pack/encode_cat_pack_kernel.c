// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/cat_pack/encode_cat_pack_kernel.h"
#include <string.h>


size_t catPackEncode(void* dest_void, const void* src_void, size_t length) {
    const uint8_t* src = (const uint8_t*)src_void;
    uint8_t* dest = (uint8_t*)dest_void;

    if (length == 0) return 0;

    // 1. Find unique values dynamically for this chunk
    uint8_t counts[256] = {0};
    for (size_t i = 0; i < length; ++i) {
        counts[src[i]] = 1;
    }
    
    uint8_t unique_vals[256];
    int num_classes = 0;
    for (int i = 0; i < 256; ++i) {
        if (counts[i]) {
            unique_vals[num_classes++] = i;
        }
    }
    
    uint8_t bits_per_val;
    if (num_classes <= 2) {
        bits_per_val = 1;
    } else if (num_classes <= 4) {
        bits_per_val = 2;
    } else if (num_classes <= 16) {
        bits_per_val = 4;
    } else {
        return 0; // Too many classes
    }

    uint8_t val_to_idx[256] = {0};
    for (int i = 0; i < num_classes; ++i) {
        val_to_idx[unique_vals[i]] = i;
    }
    
    dest[0] = num_classes;
    memcpy(dest + 1, unique_vals, num_classes);
    
    uint8_t *out = dest + 1 + num_classes;
    size_t out_len = 0;
    size_t i = 0;


    // Scalar fallback/tail processing
    uint8_t current_byte = 0;
    uint8_t bits_filled = 0;
    
    for (; i < length; ++i) {
        uint8_t idx = val_to_idx[src[i]];
        current_byte |= (idx << (8 - bits_filled - bits_per_val));
        bits_filled += bits_per_val;
        if (bits_filled >= 8) {
            out[out_len++] = current_byte;
            current_byte = 0;
            bits_filled = 0;
        }
    }
    
    if (bits_filled > 0) {
        out[out_len++] = current_byte;
    }
    
    size_t total_written = 1 + num_classes + out_len;
    // OpenZL doesn't require zero filling since it tracks exact size
    // But we might pad to byte boundaries if needed, which we already did.
    
    return total_written;
}
