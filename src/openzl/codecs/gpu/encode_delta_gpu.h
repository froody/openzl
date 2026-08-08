// Copyright (c) Meta Platforms, Inc. and affiliates.
// GPU-accelerated Delta encoding and decoding kernels using CUDA & Thrust/CUB.

#ifndef ZL_ENCODE_DELTA_GPU_H
#define ZL_ENCODE_DELTA_GPU_H

#include <cuda_runtime.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// GPU Delta 8-bit
cudaError_t ZS_deltaEncode8_gpu(
        uint8_t* d_first,
        uint8_t* d_deltas,
        uint8_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

cudaError_t ZS_deltaDecode8_gpu(
        uint8_t* d_dst,
        uint8_t first,
        uint8_t const* d_deltas,
        size_t nelts,
        cudaStream_t stream);

// GPU Delta 16-bit
cudaError_t ZS_deltaEncode16_gpu(
        uint16_t* d_first,
        uint16_t* d_deltas,
        uint16_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

cudaError_t ZS_deltaDecode16_gpu(
        uint16_t* d_dst,
        uint16_t first,
        uint16_t const* d_deltas,
        size_t nelts,
        cudaStream_t stream);

// GPU Delta 32-bit
cudaError_t ZS_deltaEncode32_gpu(
        uint32_t* d_first,
        uint32_t* d_deltas,
        uint32_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

cudaError_t ZS_deltaDecode32_gpu(
        uint32_t* d_dst,
        uint32_t first,
        uint32_t const* d_deltas,
        size_t nelts,
        cudaStream_t stream);

#ifdef __cplusplus
}
#endif

#endif // ZL_ENCODE_DELTA_GPU_H
