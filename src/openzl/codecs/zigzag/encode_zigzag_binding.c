// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/zigzag/encode_zigzag_binding.h"
#include "openzl/codecs/zigzag/encode_zigzag_kernel.h" // ZS_zigzagEncodeXX
#include "openzl/common/assertion.h"

#if OPENZL_HAS_CUDA
#    include <cuda_runtime.h>
#    include "openzl/codecs/gpu/encode_zigzag_gpu.h"
#endif

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

#if OPENZL_HAS_CUDA
    static int gpu_enabled = -1;
    if (gpu_enabled == -1) {
        char const* env = getenv("OPENZL_USE_GPU");
        gpu_enabled = (env && atoi(env) > 0) ? 1 : 0;
    }
    if (gpu_enabled && nbInts >= 65536) {
        static _Thread_local cudaStream_t tls_stream = NULL;
        if (!tls_stream) {
            cudaStreamCreateWithFlags(&tls_stream, cudaStreamNonBlocking);
        }
        void const* src = ZL_Input_ptr(in);
        void* dst       = ZL_Output_ptr(out);
        void *d_src = NULL, *d_dst = NULL;
        size_t const bytes = nbInts * numWidth;
        if (cudaMallocAsync(&d_src, bytes, tls_stream) == cudaSuccess &&
            cudaMallocAsync(&d_dst, bytes, tls_stream) == cudaSuccess) {
            cudaMemcpyAsync(d_src, src, bytes, cudaMemcpyHostToDevice, tls_stream);
            switch (numWidth) {
                case 1:
                    ZL_zigzagEncode8_gpu((uint8_t*)d_dst, (int8_t const*)d_src, nbInts, tls_stream);
                    break;
                case 2:
                    ZL_zigzagEncode16_gpu((uint16_t*)d_dst, (int16_t const*)d_src, nbInts, tls_stream);
                    break;
                case 4:
                    ZL_zigzagEncode32_gpu((uint32_t*)d_dst, (int32_t const*)d_src, nbInts, tls_stream);
                    break;
                case 8:
                    ZL_zigzagEncode64_gpu((uint64_t*)d_dst, (int64_t const*)d_src, nbInts, tls_stream);
                    break;
            }
            cudaMemcpyAsync(dst, d_dst, bytes, cudaMemcpyDeviceToHost, tls_stream);
            cudaFreeAsync(d_src, tls_stream);
            cudaFreeAsync(d_dst, tls_stream);
            cudaStreamSynchronize(tls_stream);
            ZL_ERR_IF_ERR(ZL_Output_commit(out, nbInts));
            return ZL_returnValue(1);
        }
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
