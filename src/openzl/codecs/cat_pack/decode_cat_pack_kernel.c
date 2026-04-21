// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/cat_pack/decode_cat_pack_kernel.h"
#include <string.h>


int catPackDecode(void* dest_void, const void* src_void, size_t length, size_t compressed_len) {
    const uint8_t* src = (const uint8_t*)src_void;
    uint8_t* dest = (uint8_t*)dest_void;

    if (length == 0) return 1;
    if (compressed_len == 0) return 0;

    uint8_t num_classes = src[0];
    if (num_classes == 0) {
        return 0; // Unexpected state
    }
    
    if (compressed_len < 1 + num_classes) return 0;

    uint8_t bits_per_val;
    if (num_classes <= 2) {
        bits_per_val = 1;
    } else if (num_classes <= 4) {
        bits_per_val = 2;
    } else if (num_classes <= 16) {
        bits_per_val = 4;
    } else {
        return 0;
    }

    const uint8_t *unique_vals = src + 1;
    const uint8_t *packed = src + 1 + num_classes;
    size_t packed_len = compressed_len - (1 + num_classes);
    
    size_t dest_len = 0;
    size_t i = 0;


    if (dest_len >= length) return 1;

    // Fallback unpack scalar for remainder
    if (i >= packed_len) return 0;
    
    uint8_t current_byte = packed[i];
    uint8_t bits_read = 0;
    
    for (; dest_len < length; ++dest_len) {
        uint8_t idx = (current_byte >> (8 - bits_read - bits_per_val)) & ((1 << bits_per_val) - 1);
        dest[dest_len] = unique_vals[idx];
        bits_read += bits_per_val;
        if (bits_read >= 8) {
            i++;
            if (dest_len + 1 < length && i < packed_len) {
                current_byte = packed[i];
                bits_read = 0;
            }
        }
    }
    
    return 1;
}
