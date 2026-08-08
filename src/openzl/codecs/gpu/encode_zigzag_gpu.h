// Copyright (c) Meta Platforms, Inc. and affiliates.
// GPU-accelerated Zigzag encoding and decoding kernels using CUDA.

#ifndef ZL_ENCODE_ZIGZAG_GPU_H
#define ZL_ENCODE_ZIGZAG_GPU_H

#include <cuda_runtime.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// GPU Zigzag 8-bit
cudaError_t ZL_zigzagEncode8_gpu(
        uint8_t* d_dst,
        int8_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

cudaError_t ZL_zigzagDecode8_gpu(
        int8_t* d_dst,
        uint8_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

// GPU Zigzag 16-bit
cudaError_t ZL_zigzagEncode16_gpu(
        uint16_t* d_dst,
        int16_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

cudaError_t ZL_zigzagDecode16_gpu(
        int16_t* d_dst,
        uint16_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

// GPU Zigzag 32-bit
cudaError_t ZL_zigzagEncode32_gpu(
        uint32_t* d_dst,
        int32_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

cudaError_t ZL_zigzagDecode32_gpu(
        int32_t* d_dst,
        uint32_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

// GPU Zigzag 64-bit
cudaError_t ZL_zigzagEncode64_gpu(
        uint64_t* d_dst,
        int64_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

cudaError_t ZL_zigzagDecode64_gpu(
        int64_t* d_dst,
        uint64_t const* d_src,
        size_t nelts,
        cudaStream_t stream);

#ifdef __cplusplus
}
#endif

#endif // ZL_ENCODE_ZIGZAG_GPU_H
