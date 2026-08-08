// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/delta/encode_delta_binding.h"
#include "openzl/codecs/delta/encode_delta_kernel.h" // ZS_deltaEncodeXX
#include "openzl/common/assertion.h"
#include "openzl/common/errors_internal.h"
#include "openzl/zl_data.h"

#if defined(OPENZL_HAS_CUDA) && OPENZL_HAS_CUDA
#    include <cuda_runtime.h>
#    include "openzl/codecs/gpu/encode_delta_gpu.h"
#    include "openzl/codecs/gpu/gpu_dispatch.h"

/// Delta-encodes @nelts bytes of @src into @first / @deltas on the GPU.
/// Returns false without touching either output if the payload is not worth
/// offloading or if any step failed, in which case the caller must run the CPU
/// kernel. Only the 8-bit width has a GPU kernel; wider widths stay on the CPU.
static bool deltaEncode8OnGpu(
        uint8_t* first,
        uint8_t* deltas,
        uint8_t const* src,
        size_t nelts)
{
    if (!ZL_GPU_enabled() || nelts < ZL_GPU_MIN_NB_ELTS) {
        return false;
    }
    cudaStream_t const stream = ZL_GPU_stream();
    if (stream == NULL) {
        return false;
    }

    void* d_src    = NULL;
    void* d_deltas = NULL;
    void* d_first  = NULL;

    cudaError_t err = cudaMallocAsync(&d_src, nelts, stream);
    if (err == cudaSuccess) {
        err = cudaMallocAsync(&d_deltas, nelts - 1, stream);
    }
    if (err == cudaSuccess) {
        err = cudaMallocAsync(&d_first, 1, stream);
    }
    if (err == cudaSuccess) {
        err = cudaMemcpyAsync(
                d_src, src, nelts, cudaMemcpyHostToDevice, stream);
    }
    if (err == cudaSuccess) {
        err = ZS_deltaEncode8_gpu(
                (uint8_t*)d_first,
                (uint8_t*)d_deltas,
                (uint8_t const*)d_src,
                nelts,
                stream);
    }
    if (err == cudaSuccess) {
        err = cudaMemcpyAsync(
                first, d_first, 1, cudaMemcpyDeviceToHost, stream);
    }
    if (err == cudaSuccess) {
        err = cudaMemcpyAsync(
                deltas, d_deltas, nelts - 1, cudaMemcpyDeviceToHost, stream);
    }
    if (err == cudaSuccess) {
        // Nothing may be read out of @first / @deltas before this returns, and
        // this is also where asynchronous failures are finally reported.
        err = cudaStreamSynchronize(stream);
    }

    // Frees are stream-ordered and must happen on every path, including the
    // failing ones, or the memory pool grows until the device is exhausted.
    if (d_src != NULL) {
        cudaFreeAsync(d_src, stream);
    }
    if (d_deltas != NULL) {
        cudaFreeAsync(d_deltas, stream);
    }
    if (d_first != NULL) {
        cudaFreeAsync(d_first, stream);
    }

    if (err != cudaSuccess) {
        return ZL_GPU_failed(err, "delta GPU encode");
    }
    return ZL_GPU_succeeded();
}
#endif // OPENZL_HAS_CUDA

/// Delta-encodes @nbElts elements of @src, on the GPU when it is available and
/// the payload is large enough, otherwise on the CPU. Both paths produce
/// identical bytes.
static void deltaEncode(
        void* first,
        void* deltas,
        void const* src,
        size_t nbElts,
        size_t eltWidth)
{
#if defined(OPENZL_HAS_CUDA) && OPENZL_HAS_CUDA
    if (eltWidth == 1
        && deltaEncode8OnGpu(
                (uint8_t*)first,
                (uint8_t*)deltas,
                (uint8_t const*)src,
                nbElts)) {
        return;
    }
#endif
    ZS_deltaEncode(first, deltas, src, nbElts, eltWidth);
}

// ZL_TypedEncoderFn
// This variant is compatible with any allowed integer width
ZL_Report EI_delta_int(ZL_Encoder* eictx, const ZL_Input* ins[], size_t nbIns)
{
    ZL_RESULT_DECLARE_SCOPE_REPORT(eictx);
    ZL_ASSERT_NN(eictx);
    ZL_ASSERT_EQ(nbIns, 1);
    ZL_ASSERT_NN(ins);
    const ZL_Input* in = ins[0];
    ZL_ASSERT_NN(in);
    ZL_ASSERT_EQ(ZL_Input_type(in), ZL_Type_numeric);
    size_t const intWidth = ZL_Input_eltWidth(in);
    ZL_ASSERT(intWidth == 1 || intWidth == 2 || intWidth == 4 || intWidth == 8);
    size_t const nbInts = ZL_Input_numElts(in);
    ZL_Output* const out =
            ZL_Encoder_createTypedStream(eictx, 0, nbInts, intWidth);
    ZL_ERR_IF_NULL(out, allocation);
    const void* src = ZL_Input_ptr(in);
    // Note : proper alignment is guaranteed by graph engine
    void* dst = ZL_Output_ptr(out);
    if (nbInts == 0) {
        // Special case: Zero ints behaves the same for both variants.
        ZL_ERR_IF_ERR(ZL_Output_commit(out, 0));
    } else if (ZL_Encoder_getCParam(eictx, ZL_CParam_formatVersion) < 13) {
        // Old variant: Write the first element to the first value in the stream
        deltaEncode(dst, (char*)dst + intWidth, src, nbInts, intWidth);
        ZL_ERR_IF_ERR(ZL_Output_commit(out, nbInts));
    } else {
        // New variant: Write the first element to the stream header
        uint8_t header[8];
        ZL_ASSERT_GT(nbInts, 0);
        ZL_ASSERT_LE(intWidth, sizeof(header));
        deltaEncode(header, dst, src, nbInts, intWidth);
        ZL_Encoder_sendCodecHeader(eictx, header, intWidth);
        ZL_ERR_IF_ERR(ZL_Output_commit(out, nbInts - 1));
    }
    return ZL_returnValue(1);
}
