// Copyright (c) Meta Platforms, Inc. and affiliates.
// CUDA implementations for GPU Zigzag encoding/decoding kernels.

#include "encode_zigzag_gpu.h"

namespace {

constexpr int kThreadsPerBlock = 256;

// Grids are capped and every kernel below is a grid-stride loop, so element
// counts beyond what a single thread-per-element launch could address are
// still handled correctly.
constexpr size_t kMaxBlocks = 65535;

unsigned int blocksFor(size_t nelts)
{
    size_t blocks = (nelts + kThreadsPerBlock - 1) / kThreadsPerBlock;
    if (blocks == 0) {
        blocks = 1;
    }
    if (blocks > kMaxBlocks) {
        blocks = kMaxBlocks;
    }
    return (unsigned int)blocks;
}

// blockIdx/blockDim are 32-bit; the products below are widened to size_t
// before they can wrap on payloads larger than 4G elements.
__device__ inline size_t globalIndex()
{
    return (size_t)blockIdx.x * blockDim.x + threadIdx.x;
}

__device__ inline size_t globalStride()
{
    return (size_t)blockDim.x * gridDim.x;
}

__global__ void zigzag_encode8_kernel(uint8_t* dst, int8_t const* src, size_t nelts)
{
    for (size_t idx = globalIndex(); idx < nelts; idx += globalStride()) {
        int32_t const n = src[idx];
        dst[idx]        = (uint8_t)(((uint8_t)n << 1) ^ (uint8_t)(n >> 7));
    }
}

__global__ void zigzag_decode8_kernel(int8_t* dst, uint8_t const* src, size_t nelts)
{
    for (size_t idx = globalIndex(); idx < nelts; idx += globalStride()) {
        uint8_t const z    = src[idx];
        uint8_t const mask = (uint8_t)((uint8_t)0 - (uint8_t)(z & 0x1));
        dst[idx]           = (int8_t)((z >> 1) ^ mask);
    }
}

__global__ void zigzag_encode16_kernel(uint16_t* dst, int16_t const* src, size_t nelts)
{
    for (size_t idx = globalIndex(); idx < nelts; idx += globalStride()) {
        int16_t const n = src[idx];
        dst[idx]        = (uint16_t)(((uint16_t)n << 1) ^ (uint16_t)(n >> 15));
    }
}

__global__ void zigzag_decode16_kernel(int16_t* dst, uint16_t const* src, size_t nelts)
{
    for (size_t idx = globalIndex(); idx < nelts; idx += globalStride()) {
        uint16_t const z    = src[idx];
        uint16_t const mask = (uint16_t)((uint16_t)0 - (uint16_t)(z & 0x1));
        dst[idx]            = (int16_t)((z >> 1) ^ mask);
    }
}

__global__ void zigzag_encode32_kernel(uint32_t* dst, int32_t const* src, size_t nelts)
{
    for (size_t idx = globalIndex(); idx < nelts; idx += globalStride()) {
        int32_t const n = src[idx];
        dst[idx]        = ((uint32_t)n << 1) ^ (uint32_t)(n >> 31);
    }
}

__global__ void zigzag_decode32_kernel(int32_t* dst, uint32_t const* src, size_t nelts)
{
    for (size_t idx = globalIndex(); idx < nelts; idx += globalStride()) {
        uint32_t const z    = src[idx];
        uint32_t const mask = 0U - (z & 0x1);
        dst[idx]            = (int32_t)((z >> 1) ^ mask);
    }
}

__global__ void zigzag_encode64_kernel(uint64_t* dst, int64_t const* src, size_t nelts)
{
    for (size_t idx = globalIndex(); idx < nelts; idx += globalStride()) {
        int64_t const n = src[idx];
        dst[idx]        = ((uint64_t)n << 1) ^ (uint64_t)(n >> 63);
    }
}

__global__ void zigzag_decode64_kernel(int64_t* dst, uint64_t const* src, size_t nelts)
{
    for (size_t idx = globalIndex(); idx < nelts; idx += globalStride()) {
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
    zigzag_encode8_kernel<<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
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
    zigzag_decode8_kernel<<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
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
    zigzag_encode16_kernel<<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
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
    zigzag_decode16_kernel<<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
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
    zigzag_encode32_kernel<<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
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
    zigzag_decode32_kernel<<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
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
    zigzag_encode64_kernel<<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
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
    zigzag_decode64_kernel<<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
            d_dst, d_src, nelts);
    return cudaGetLastError();
}

} // extern "C"
