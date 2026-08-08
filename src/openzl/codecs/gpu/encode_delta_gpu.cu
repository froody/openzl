// Copyright (c) Meta Platforms, Inc. and affiliates.
// CUDA implementations for GPU Delta encoding/decoding kernels.

#include "encode_delta_gpu.h"

#include <thrust/execution_policy.h>
#include <thrust/scan.h>

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

// ----------------------------------------------------------------------------
// Delta Encode CUDA Kernels
// ----------------------------------------------------------------------------

template <typename T>
__global__ void delta_encode_kernel(T* deltas, T const* src, size_t nelts)
{
    // Element 0 has no predecessor; it is carried separately as `first`.
    for (size_t idx = globalIndex() + 1; idx < nelts; idx += globalStride()) {
        deltas[idx - 1] = (T)(src[idx] - src[idx - 1]);
    }
}

template <typename T>
__global__ void delta_decode_add_first_kernel(T* dst, T first, size_t nelts)
{
    for (size_t idx = globalIndex(); idx < nelts; idx += globalStride()) {
        dst[idx] = (T)(dst[idx] + first);
    }
}

/// Runs the prefix scan that inverts the delta encode. Thrust's temporary
/// allocation throws on failure; these functions are `extern "C"` and are
/// called from C, so the exception must not be allowed to escape.
template <typename T>
cudaError_t inclusiveScan(T const* d_deltas, T* d_out, size_t nelts, cudaStream_t stream)
{
    try {
        thrust::inclusive_scan(
                thrust::cuda::par_nosync.on(stream),
                d_deltas,
                d_deltas + nelts,
                d_out);
    } catch (...) {
        // Surface it as a normal CUDA failure so the caller falls back to CPU.
        cudaGetLastError();
        return cudaErrorMemoryAllocation;
    }
    return cudaGetLastError();
}

} // namespace

extern "C" {

cudaError_t ZS_deltaEncode8_gpu(
        uint8_t* d_first,
        uint8_t* d_deltas,
        uint8_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;

    // Copy first element
    cudaError_t err = cudaMemcpyAsync(
            d_first, d_src, sizeof(uint8_t), cudaMemcpyDeviceToDevice, stream);
    if (err != cudaSuccess) return err;

    if (nelts > 1) {
        delta_encode_kernel<uint8_t>
                <<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
                        d_deltas, d_src, nelts);
    }
    return cudaGetLastError();
}

cudaError_t ZS_deltaDecode8_gpu(
        uint8_t* d_dst,
        uint8_t first,
        uint8_t const* d_deltas,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;

    // Set first element
    uint8_t h_first = first;
    cudaError_t err = cudaMemcpyAsync(
            d_dst, &h_first, sizeof(uint8_t), cudaMemcpyHostToDevice, stream);
    if (err != cudaSuccess) return err;

    if (nelts > 1) {
        err = inclusiveScan(d_deltas, d_dst + 1, nelts - 1, stream);
        if (err != cudaSuccess) return err;

        delta_decode_add_first_kernel<uint8_t>
                <<<blocksFor(nelts - 1), kThreadsPerBlock, 0, stream>>>(
                        d_dst + 1, first, nelts - 1);
    }
    return cudaGetLastError();
}

cudaError_t ZS_deltaEncode16_gpu(
        uint16_t* d_first,
        uint16_t* d_deltas,
        uint16_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;

    cudaError_t err = cudaMemcpyAsync(
            d_first, d_src, sizeof(uint16_t), cudaMemcpyDeviceToDevice, stream);
    if (err != cudaSuccess) return err;

    if (nelts > 1) {
        delta_encode_kernel<uint16_t>
                <<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
                        d_deltas, d_src, nelts);
    }
    return cudaGetLastError();
}

cudaError_t ZS_deltaDecode16_gpu(
        uint16_t* d_dst,
        uint16_t first,
        uint16_t const* d_deltas,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;

    uint16_t h_first = first;
    cudaError_t err  = cudaMemcpyAsync(
            d_dst, &h_first, sizeof(uint16_t), cudaMemcpyHostToDevice, stream);
    if (err != cudaSuccess) return err;

    if (nelts > 1) {
        err = inclusiveScan(d_deltas, d_dst + 1, nelts - 1, stream);
        if (err != cudaSuccess) return err;

        delta_decode_add_first_kernel<uint16_t>
                <<<blocksFor(nelts - 1), kThreadsPerBlock, 0, stream>>>(
                        d_dst + 1, first, nelts - 1);
    }
    return cudaGetLastError();
}

cudaError_t ZS_deltaEncode32_gpu(
        uint32_t* d_first,
        uint32_t* d_deltas,
        uint32_t const* d_src,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;

    cudaError_t err = cudaMemcpyAsync(
            d_first, d_src, sizeof(uint32_t), cudaMemcpyDeviceToDevice, stream);
    if (err != cudaSuccess) return err;

    if (nelts > 1) {
        delta_encode_kernel<uint32_t>
                <<<blocksFor(nelts), kThreadsPerBlock, 0, stream>>>(
                        d_deltas, d_src, nelts);
    }
    return cudaGetLastError();
}

cudaError_t ZS_deltaDecode32_gpu(
        uint32_t* d_dst,
        uint32_t first,
        uint32_t const* d_deltas,
        size_t nelts,
        cudaStream_t stream)
{
    if (nelts == 0) return cudaSuccess;

    uint32_t h_first = first;
    cudaError_t err  = cudaMemcpyAsync(
            d_dst, &h_first, sizeof(uint32_t), cudaMemcpyHostToDevice, stream);
    if (err != cudaSuccess) return err;

    if (nelts > 1) {
        err = inclusiveScan(d_deltas, d_dst + 1, nelts - 1, stream);
        if (err != cudaSuccess) return err;

        delta_decode_add_first_kernel<uint32_t>
                <<<blocksFor(nelts - 1), kThreadsPerBlock, 0, stream>>>(
                        d_dst + 1, first, nelts - 1);
    }
    return cudaGetLastError();
}

} // extern "C"
