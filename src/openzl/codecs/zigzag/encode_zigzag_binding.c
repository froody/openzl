// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/zigzag/encode_zigzag_binding.h"
#include "openzl/codecs/zigzag/encode_zigzag_kernel.h" // ZS_zigzagEncodeXX
#include "openzl/common/assertion.h"

#if defined(OPENZL_HAS_CUDA) && OPENZL_HAS_CUDA
#    include <cuda_runtime.h>
#    include "openzl/codecs/gpu/encode_zigzag_gpu.h"
#    include "openzl/codecs/gpu/gpu_dispatch.h"

static cudaError_t zigzagEncode_gpu(
        void* d_dst,
        void const* d_src,
        size_t nbInts,
        size_t numWidth,
        cudaStream_t stream)
{
    switch (numWidth) {
        case 1:
            return ZL_zigzagEncode8_gpu(
                    (uint8_t*)d_dst, (int8_t const*)d_src, nbInts, stream);
        case 2:
            return ZL_zigzagEncode16_gpu(
                    (uint16_t*)d_dst, (int16_t const*)d_src, nbInts, stream);
        case 4:
            return ZL_zigzagEncode32_gpu(
                    (uint32_t*)d_dst, (int32_t const*)d_src, nbInts, stream);
        case 8:
            return ZL_zigzagEncode64_gpu(
                    (uint64_t*)d_dst, (int64_t const*)d_src, nbInts, stream);
        default:
            // Unreachable: callers filter widths via zigzagWidthHasGpuKernel().
            return cudaErrorInvalidValue;
    }
}

static bool zigzagWidthHasGpuKernel(size_t numWidth)
{
    return numWidth == 1 || numWidth == 2 || numWidth == 4 || numWidth == 8;
}

/// Zigzag-encodes @nbInts elements of @src into @dst on the GPU.
/// Returns false without touching @dst if the payload is not worth offloading
/// or if any step failed, in which case the caller must run the CPU kernel.
static bool zigzagEncodeOnGpu(
        void* dst,
        void const* src,
        size_t nbInts,
        size_t numWidth)
{
    if (!ZL_GPU_enabled() || nbInts < ZL_GPU_MIN_NB_ELTS
        || !zigzagWidthHasGpuKernel(numWidth)) {
        return false;
    }
    cudaStream_t const stream = ZL_GPU_stream();
    if (stream == NULL) {
        return false;
    }

    size_t const bytes = nbInts * numWidth;
    void* d_src        = NULL;
    void* d_dst        = NULL;

    cudaError_t err = cudaMallocAsync(&d_src, bytes, stream);
    if (err == cudaSuccess) {
        err = cudaMallocAsync(&d_dst, bytes, stream);
    }
    if (err == cudaSuccess) {
        err = cudaMemcpyAsync(
                d_src, src, bytes, cudaMemcpyHostToDevice, stream);
    }
    if (err == cudaSuccess) {
        err = zigzagEncode_gpu(d_dst, d_src, nbInts, numWidth, stream);
    }
    if (err == cudaSuccess) {
        err = cudaMemcpyAsync(
                dst, d_dst, bytes, cudaMemcpyDeviceToHost, stream);
    }
    if (err == cudaSuccess) {
        // Nothing may be read out of @dst before this returns, and this is
        // also where asynchronous failures (a failed launch, a stream-ordered
        // allocation that could not be honoured) are finally reported.
        err = cudaStreamSynchronize(stream);
    }

    // Frees are stream-ordered and must happen on every path, including the
    // failing ones, or the memory pool grows until the device is exhausted.
    if (d_src != NULL) {
        cudaFreeAsync(d_src, stream);
    }
    if (d_dst != NULL) {
        cudaFreeAsync(d_dst, stream);
    }

    if (err != cudaSuccess) {
        return ZL_GPU_failed(err, "zigzag GPU encode");
    }
    return ZL_GPU_succeeded();
}
#endif // OPENZL_HAS_CUDA

// ZL_TypedEncoderFn
ZL_Report EI_zigzag_num(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns)
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(eictx);
    ZL_ASSERT_NN(eictx);
    ZL_ASSERT_EQ(nbIns, 1);
    ZL_ASSERT_NN(ins);
    const ZL_Input* const in = ins[0];
    ZL_ASSERT_NN(in);
    ZL_ASSERT_EQ(ZL_Input_type(in), ZL_Type_numeric);
    size_t const numWidth = ZL_Input_eltWidth(in);
    size_t const nbInts   = ZL_Input_numElts(in);
    ZL_Output* const out =
            ZL_Encoder_createTypedStream(eictx, 0, nbInts, numWidth);
    ZL_ERR_IF_NULL(out, allocation);
    ZL_ASSERT(numWidth == 1 || numWidth == 2 || numWidth == 4 || numWidth == 8);

#if defined(OPENZL_HAS_CUDA) && OPENZL_HAS_CUDA
    if (zigzagEncodeOnGpu(
                ZL_Output_ptr(out), ZL_Input_ptr(in), nbInts, numWidth)) {
        ZL_ERR_IF_ERR(ZL_Output_commit(out, nbInts));
        return ZL_returnValue(1);
    }
#endif

    switch (numWidth) {
        case 1:
            ZL_zigzagEncode8(ZL_Output_ptr(out), ZL_Input_ptr(in), nbInts);
            break;
        case 2:
            ZL_zigzagEncode16(ZL_Output_ptr(out), ZL_Input_ptr(in), nbInts);
            break;
        case 4:
            ZL_zigzagEncode32(ZL_Output_ptr(out), ZL_Input_ptr(in), nbInts);
            break;
        case 8:
            ZL_zigzagEncode64(ZL_Output_ptr(out), ZL_Input_ptr(in), nbInts);
            break;
        default:
            ZL_ASSERT_FAIL("Unreachable");
    }
    ZL_ERR_IF_ERR(ZL_Output_commit(out, nbInts));
    return ZL_returnValue(1);
}
