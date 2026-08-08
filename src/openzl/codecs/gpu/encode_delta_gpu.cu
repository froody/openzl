// Copyright (c) Meta Platforms, Inc. and affiliates.
// CUDA implementations for GPU Delta encoding/decoding kernels.

#include "encode_delta_gpu.h"

#include <thrust/execution_policy.h>
#include <thrust/scan.h>

namespace {

// ----------------------------------------------------------------------------
// Delta Encode CUDA Kernels
// ----------------------------------------------------------------------------

template <typename T>
__global__ void delta_encode_kernel(T* deltas, T const* src, size_t nelts)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx > 0 && idx < nelts) {
        deltas[idx - 1] = src[idx] - src[idx - 1];
    }
}

template <typename T>
__global__ void delta_decode_add_first_kernel(T* dst, T first, size_t nelts)
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < nelts) {
        dst[idx] += first;
    }
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
        int threadsPerBlock = 256;
        int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
        delta_encode_kernel<uint8_t><<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
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
        // Parallel inclusive prefix scan over deltas
        thrust::inclusive_scan(
                thrust::cuda::par.on(stream),
                d_deltas,
                d_deltas + (nelts - 1),
                d_dst + 1);

        // Add 'first' to all elements
        int threadsPerBlock = 256;
        int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
        delta_decode_add_first_kernel<uint8_t><<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
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
        int threadsPerBlock = 256;
        int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
        delta_encode_kernel<uint16_t><<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
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
        thrust::inclusive_scan(
                thrust::cuda::par.on(stream),
                d_deltas,
                d_deltas + (nelts - 1),
                d_dst + 1);

        int threadsPerBlock = 256;
        int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
        delta_decode_add_first_kernel<uint16_t><<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
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
        int threadsPerBlock = 256;
        int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
        delta_encode_kernel<uint32_t><<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
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
        thrust::inclusive_scan(
                thrust::cuda::par.on(stream),
                d_deltas,
                d_deltas + (nelts - 1),
                d_dst + 1);

        int threadsPerBlock = 256;
        int blocksPerGrid   = (int)((nelts + threadsPerBlock - 1) / threadsPerBlock);
        delta_decode_add_first_kernel<uint32_t><<<blocksPerGrid, threadsPerBlock, 0, stream>>>(
                d_dst + 1, first, nelts - 1);
    }
    return cudaGetLastError();
}

} // extern "C"
