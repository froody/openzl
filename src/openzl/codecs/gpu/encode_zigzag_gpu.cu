// Copyright (c) Meta Platforms, Inc. and affiliates.
// CUDA implementations for GPU Zigzag encoding/decoding kernels.

#include "encode_zigzag_gpu.h"

namespace {

__global__ void zigzag_encode8_kernel(uint8_t* dst, int8_t const* src, size_t nelts)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < nelts) {
        int32_t const n = src[idx];
        dst[idx]        = (uint8_t)(((uint8_t)n << 1) ^ (uint8_t)(n >> 7));
    }
}

__global__ void zigzag_decode8_kernel(int8_t* dst, uint8_t const* src, size_t nelts)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < nelts) {
        uint8_t const z    = src[idx];
        uint8_t const mask = (uint8_t)0 - (z & 0x1);
        dst[idx]           = (int8_t)((z >> 1) ^ mask);
    }
}

__global__ void zigzag_encode16_kernel(uint16_t* dst, int16_t const* src, size_t nelts)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < nelts) {
        int16_t const n = src[idx];
        dst[idx]        = (uint16_t)(((uint16_t)n << 1) ^ (uint16_t)(n >> 15));
    }
}

__global__ void zigzag_decode16_kernel(int16_t* dst, uint16_t const* src, size_t nelts)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < nelts) {
        uint16_t const z    = src[idx];
        uint16_t const mask = (uint16_t)0 - (z & 0x1);
        dst[idx]            = (int16_t)((z >> 1) ^ mask);
    }
}

__global__ void zigzag_encode32_kernel(uint32_t* dst, int32_t const* src, size_t nelts)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < nelts) {
        int32_t const n = src[idx];
        dst[idx]        = ((uint32_t)n << 1) ^ (uint32_t)(n >> 31);
    }
}

__global__ void zigzag_decode32_kernel(int32_t* dst, uint32_t const* src, size_t nelts)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < nelts) {
        uint32_t const z    = src[idx];
        uint32_t const mask = 0U - (z & 0x1);
        dst[idx]            = (int32_t)((z >> 1) ^ mask);
    }
}

__global__ void zigzag_encode64_kernel(uint64_t* dst, int64_t const* src, size_t nelts)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < nelts) {
        int64_t const n = src[idx];
        dst[idx]        = ((uint64_t)n << 1) ^ (uint64_t)(n >> 63);
    }
}

__global__ void zigzag_decode64_kernel(int64_t* dst, uint64_t const* src, size_t nelts)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < nelts) {
        uint64_t const z    = src[idx];
        uint64_t const mask = 0ULL - (z & 0x1);
        dst[idx]            = (int64_t)((z >> 1) ^ mask);
    }
}

} // namespace

extern "C" {

cudaError_t ZL_zigzagEncode8_gpu(
        uint8_t* d_dst,
        int8_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;
    int threadsPerBlock = 256;
    int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
    zigzag_encode8_kernel<<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
            d_dst, d_src, nelts);
    return cudaGetLastError();
}

cudaError_t ZL_zigzagDecode8_gpu(
        int8_t* d_dst,
        uint8_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;
    int threadsPerBlock = 256;
    int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
    zigzag_decode8_kernel<<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
            d_dst, d_src, nelts);
    return cudaGetLastError();
}

cudaError_t ZL_zigzagEncode16_gpu(
        uint16_t* d_dst,
        int16_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;
    int threadsPerBlock = 256;
    int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
    zigzag_encode16_kernel<<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
            d_dst, d_src, nelts);
    return cudaGetLastError();
}

cudaError_t ZL_zigzagDecode16_gpu(
        int16_t* d_dst,
        uint16_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;
    int threadsPerBlock = 256;
    int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
    zigzag_decode16_kernel<<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
            d_dst, d_src, nelts);
    return cudaGetLastError();
}

cudaError_t ZL_zigzagEncode32_gpu(
        uint32_t* d_dst,
        int32_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;
    int threadsPerBlock = 256;
    int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
    zigzag_encode32_kernel<<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
            d_dst, d_src, nelts);
    return cudaGetLastError();
}

cudaError_t ZL_zigzagDecode32_gpu(
        int32_t* d_dst,
        uint32_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;
    int threadsPerBlock = 256;
    int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
    zigzag_decode32_kernel<<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
            d_dst, d_src, nelts);
    return cudaGetLastError();
}

cudaError_t ZL_zigzagEncode64_gpu(
        uint64_t* d_dst,
        int64_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;
    int threadsPerBlock = 256;
    int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
    zigzag_encode64_kernel<<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
            d_dst, d_src, nelts);
    return cudaGetLastError();
}

cudaError_t ZL_zigzagDecode64_gpu(
        int64_t* d_dst,
        uint64_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;
    int threadsPerBlock = 256;
    int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
    zigzag_decode64_kernel<<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
            d_dst, d_src, nelts);
    return cudaGetLastError();
}

} // extern "C"
