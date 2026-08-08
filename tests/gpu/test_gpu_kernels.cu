// Copyright (c) Meta Platforms, Inc. and affiliates.
// GPU kernel verification & benchmark for OpenZL Delta & Zigzag CUDA kernels.

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "openzl/codecs/delta/decode_delta_kernel.h"
#include "openzl/codecs/delta/encode_delta_kernel.h"
#include "openzl/codecs/gpu/encode_delta_gpu.h"
#include "openzl/codecs/gpu/encode_zigzag_gpu.h"
#include "openzl/codecs/zigzag/decode_zigzag_kernel.h"
#include "openzl/codecs/zigzag/encode_zigzag_kernel.h"

#define CUDA_CHECK(call)                                                      \
    do {                                                                      \
        cudaError_t err = (call);                                             \
        if (err != cudaSuccess) {                                             \
            std::fprintf(                                                     \
                    stderr,                                                   \
                    "CUDA Error: %s at %s:%d\n",                              \
                    cudaGetErrorString(err),                                  \
                    __FILE__,                                                 \
                    __LINE__);                                                \
            std::exit(EXIT_FAILURE);                                          \
        }                                                                     \
    } while (0)

int main()
{
    std::printf("=== OpenZL GPU Accelerator Kernel Verification & Benchmark ===\n");

    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    std::printf("GPU Device: %s (Compute %d.%d, SMs: %d)\n\n", prop.name, prop.major, prop.minor, prop.multiProcessorCount);

    size_t const N = 100 * 1024 * 1024; // 100 million 8-bit elements (~100 MB)
    std::printf("Testing with dataset size N = %zu elements (%.1f MB)...\n", N, N / (1024.0 * 1024.0));

    std::vector<uint8_t> h_src8(N);
    for (size_t i = 0; i < N; ++i) {
        h_src8[i] = static_cast<uint8_t>(i * 31 + 17);
    }

    // Allocate GPU memory
    uint8_t *d_src8, *d_first8, *d_deltas8, *d_zigzag8, *d_decoded8;
    CUDA_CHECK(cudaMalloc(&d_src8, N));
    CUDA_CHECK(cudaMalloc(&d_first8, 1));
    CUDA_CHECK(cudaMalloc(&d_deltas8, N - 1));
    CUDA_CHECK(cudaMalloc(&d_zigzag8, N));
    CUDA_CHECK(cudaMalloc(&d_decoded8, N));

    CUDA_CHECK(cudaMemcpy(d_src8, h_src8.data(), N, cudaMemcpyHostToDevice));

    // ------------------------------------------------------------------------
    // 1. GPU Delta 8-bit Test
    // ------------------------------------------------------------------------
    std::printf("Testing GPU Delta 8-bit Encode & Decode...\n");
    CUDA_CHECK(ZS_deltaEncode8_gpu(d_first8, d_deltas8, d_src8, N, 0));
    uint8_t h_first = 0;
    CUDA_CHECK(cudaMemcpy(&h_first, d_first8, 1, cudaMemcpyDeviceToHost));
    CUDA_CHECK(ZS_deltaDecode8_gpu(d_decoded8, h_first, d_deltas8, N, 0));
    CUDA_CHECK(cudaDeviceSynchronize());

    std::vector<uint8_t> h_decoded8(N);
    CUDA_CHECK(cudaMemcpy(h_decoded8.data(), d_decoded8, N, cudaMemcpyDeviceToHost));

    if (std::memcmp(h_src8.data(), h_decoded8.data(), N) == 0) {
        std::printf("[PASS] GPU Delta 8-bit round-trip verified (100MB exact match)!\n");
    } else {
        std::fprintf(stderr, "[FAIL] GPU Delta 8-bit mismatch!\n");
        return EXIT_FAILURE;
    }

    // ------------------------------------------------------------------------
    // 2. GPU Zigzag 8-bit Test
    // ------------------------------------------------------------------------
    std::printf("Testing GPU Zigzag 8-bit Encode & Decode...\n");
    CUDA_CHECK(ZL_zigzagEncode8_gpu(d_zigzag8, reinterpret_cast<int8_t const*>(d_src8), N, 0));
    CUDA_CHECK(ZL_zigzagDecode8_gpu(reinterpret_cast<int8_t*>(d_decoded8), d_zigzag8, N, 0));
    CUDA_CHECK(cudaDeviceSynchronize());

    CUDA_CHECK(cudaMemcpy(h_decoded8.data(), d_decoded8, N, cudaMemcpyDeviceToHost));

    if (std::memcmp(h_src8.data(), h_decoded8.data(), N) == 0) {
        std::printf("[PASS] GPU Zigzag 8-bit round-trip verified (100MB exact match)!\n");
    } else {
        std::fprintf(stderr, "[FAIL] GPU Zigzag 8-bit mismatch!\n");
        return EXIT_FAILURE;
    }

    // ------------------------------------------------------------------------
    // 3. Performance Benchmark: CPU SIMD vs GPU CUDA
    // ------------------------------------------------------------------------
    std::printf("\n=== Performance Benchmark (100 MB payload) ===\n");
    std::vector<uint8_t> h_deltas8(N - 1);
    uint8_t c_first = 0;

    // CPU AVX2 SIMD Delta
    auto start_cpu = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 10; ++iter) {
        ZS_deltaEncode8(&c_first, h_deltas8.data(), h_src8.data(), N);
    }
    auto end_cpu = std::chrono::high_resolution_clock::now();
    double cpu_time_ms = std::chrono::duration<double, std::milli>(end_cpu - start_cpu).count() / 10.0;
    double cpu_gb_s = (N / (1024.0 * 1024.0 * 1024.0)) / (cpu_time_ms / 1000.0);
    std::printf("CPU AVX2 Delta Encode : %6.2f ms (%.2f GB/s)\n", cpu_time_ms, cpu_gb_s);

    // GPU CUDA Delta
    // Warmup
    ZS_deltaEncode8_gpu(d_first8, d_deltas8, d_src8, N, 0);
    CUDA_CHECK(cudaDeviceSynchronize());

    auto start_gpu = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 10; ++iter) {
        ZS_deltaEncode8_gpu(d_first8, d_deltas8, d_src8, N, 0);
    }
    CUDA_CHECK(cudaDeviceSynchronize());
    auto end_gpu = std::chrono::high_resolution_clock::now();
    double gpu_time_ms = std::chrono::duration<double, std::milli>(end_gpu - start_gpu).count() / 10.0;
    double gpu_gb_s = (N / (1024.0 * 1024.0 * 1024.0)) / (gpu_time_ms / 1000.0);
    std::printf("GPU CUDA Delta Encode : %6.2f ms (%.2f GB/s)\n", gpu_time_ms, gpu_gb_s);
    std::printf("GPU Speedup over AVX2 : %.2fx\n\n", cpu_time_ms / gpu_time_ms);

    // CPU AVX2 SIMD Zigzag
    std::vector<uint8_t> h_zigzag8(N);
    start_cpu = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 10; ++iter) {
        ZL_zigzagEncode8(h_zigzag8.data(), reinterpret_cast<int8_t const*>(h_src8.data()), N);
    }
    end_cpu = std::chrono::high_resolution_clock::now();
    cpu_time_ms = std::chrono::duration<double, std::milli>(end_cpu - start_cpu).count() / 10.0;
    cpu_gb_s = (N / (1024.0 * 1024.0 * 1024.0)) / (cpu_time_ms / 1000.0);
    std::printf("CPU AVX2 Zigzag Encode : %6.2f ms (%.2f GB/s)\n", cpu_time_ms, cpu_gb_s);

    // GPU CUDA Zigzag
    ZL_zigzagEncode8_gpu(d_zigzag8, reinterpret_cast<int8_t const*>(d_src8), N, 0);
    CUDA_CHECK(cudaDeviceSynchronize());

    start_gpu = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 10; ++iter) {
        ZL_zigzagEncode8_gpu(d_zigzag8, reinterpret_cast<int8_t const*>(d_src8), N, 0);
    }
    CUDA_CHECK(cudaDeviceSynchronize());
    end_gpu = std::chrono::high_resolution_clock::now();
    gpu_time_ms = std::chrono::duration<double, std::milli>(end_gpu - start_gpu).count() / 10.0;
    gpu_gb_s = (N / (1024.0 * 1024.0 * 1024.0)) / (gpu_time_ms / 1000.0);
    std::printf("GPU CUDA Zigzag Encode : %6.2f ms (%.2f GB/s)\n", gpu_time_ms, gpu_gb_s);
    std::printf("GPU Speedup over AVX2  : %.2fx\n", cpu_time_ms / gpu_time_ms);

    // Cleanup
    CUDA_CHECK(cudaFree(d_src8));
    CUDA_CHECK(cudaFree(d_first8));
    CUDA_CHECK(cudaFree(d_deltas8));
    CUDA_CHECK(cudaFree(d_zigzag8));
    CUDA_CHECK(cudaFree(d_decoded8));

    std::printf("\n=== All GPU Accelerator Verification Tests Passed Successfully! ===\n");
    return EXIT_SUCCESS;
}
