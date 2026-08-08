// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "openzl/codecs/gpu/gpu_dispatch.h"

#if defined(OPENZL_HAS_CUDA) && OPENZL_HAS_CUDA

#    include <pthread.h>
#    include <stdatomic.h>
#    include <stdio.h>
#    include <stdlib.h>

// ----------------------------------------------------------------------------
// Enablement
// ----------------------------------------------------------------------------

static int g_gpuRequested;
static int g_gpuDebug;
static pthread_once_t g_envOnce = PTHREAD_ONCE_INIT;

// Latched off by ZL_GPU_failed() when a dispatch fails for a reason that
// suggests the CUDA context itself is unusable. Read on every dispatch.
static atomic_int g_gpuDisabled = 0;

// Dispatch tallies, reported at exit under OPENZL_GPU_DEBUG. Purely
// diagnostic: nothing in the codecs branches on these.
static atomic_long g_gpuDispatches   = 0;
static atomic_long g_gpuFallbacks    = 0;
static pthread_once_t g_reportOnce   = PTHREAD_ONCE_INIT;

static void ZL_GPU_reportAtExit(void)
{
    fprintf(stderr,
            "openzl: GPU dispatches=%ld cpu-fallbacks=%ld\n",
            atomic_load(&g_gpuDispatches),
            atomic_load(&g_gpuFallbacks));
}

static void ZL_GPU_installReport(void)
{
    atexit(ZL_GPU_reportAtExit);
}

static void ZL_GPU_readEnv(void)
{
    char const* const use = getenv("OPENZL_USE_GPU");
    g_gpuRequested        = (use != NULL && atoi(use) > 0);
    g_gpuDebug            = (getenv("OPENZL_GPU_DEBUG") != NULL);
}

static bool ZL_GPU_debugEnabled(void)
{
    pthread_once(&g_envOnce, ZL_GPU_readEnv);
    return g_gpuDebug != 0;
}

bool ZL_GPU_enabled(void)
{
    pthread_once(&g_envOnce, ZL_GPU_readEnv);
    if (!g_gpuRequested) {
        return false;
    }
    return atomic_load_explicit(&g_gpuDisabled, memory_order_relaxed) == 0;
}

// ----------------------------------------------------------------------------
// Per-thread streams
// ----------------------------------------------------------------------------

static pthread_key_t g_streamKey;
static int g_streamKeyValid;
static pthread_once_t g_streamKeyOnce = PTHREAD_ONCE_INIT;

static void ZL_GPU_destroyStream(void* stream)
{
    if (stream != NULL) {
        cudaStreamDestroy((cudaStream_t)stream);
    }
}

static void ZL_GPU_createStreamKey(void)
{
    g_streamKeyValid = (pthread_key_create(&g_streamKey, ZL_GPU_destroyStream) == 0);
}

cudaStream_t ZL_GPU_stream(void)
{
    pthread_once(&g_streamKeyOnce, ZL_GPU_createStreamKey);
    if (!g_streamKeyValid) {
        return NULL;
    }
    cudaStream_t stream = (cudaStream_t)pthread_getspecific(g_streamKey);
    if (stream != NULL) {
        return stream;
    }
    cudaError_t const err =
            cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking);
    if (err != cudaSuccess) {
        ZL_GPU_failed(err, "cudaStreamCreateWithFlags");
        return NULL;
    }
    if (pthread_setspecific(g_streamKey, stream) != 0) {
        cudaStreamDestroy(stream);
        return NULL;
    }
    return stream;
}

// ----------------------------------------------------------------------------
// Failure handling
// ----------------------------------------------------------------------------

bool ZL_GPU_succeeded(void)
{
    if (ZL_GPU_debugEnabled()) {
        pthread_once(&g_reportOnce, ZL_GPU_installReport);
        atomic_fetch_add_explicit(&g_gpuDispatches, 1, memory_order_relaxed);
    }
    return true;
}

bool ZL_GPU_failed(cudaError_t err, char const* what)
{
    if (ZL_GPU_debugEnabled()) {
        pthread_once(&g_reportOnce, ZL_GPU_installReport);
        atomic_fetch_add_explicit(&g_gpuFallbacks, 1, memory_order_relaxed);
    }
    // Drop whatever error is pending on this thread. Without this, a failure
    // here would be re-reported by the next unrelated cudaGetLastError().
    cudaGetLastError();

    // Running out of device memory is expected when many threads dispatch at
    // once; the CPU kernel handles this payload and the next one may well fit.
    // Anything else (a dead context, a missing kernel image, an illegal
    // access) will not fix itself, so stop dispatching to the GPU entirely
    // rather than paying the failed round-trip on every subsequent codec call.
    bool const transient = (err == cudaErrorMemoryAllocation);
    if (!transient) {
        int const wasDisabled =
                atomic_exchange_explicit(&g_gpuDisabled, 1, memory_order_relaxed);
        if (!wasDisabled) {
            fprintf(stderr,
                    "openzl: GPU acceleration disabled after %s failed: %s (%d). "
                    "Falling back to CPU codecs.\n",
                    what,
                    cudaGetErrorString(err),
                    (int)err);
        }
    } else if (ZL_GPU_debugEnabled()) {
        fprintf(stderr,
                "openzl: GPU dispatch fell back to CPU, %s failed: %s (%d)\n",
                what,
                cudaGetErrorString(err),
                (int)err);
    }
    return false;
}

#else // !OPENZL_HAS_CUDA

typedef int ZL_GPU_dispatchEmptyTranslationUnit;

#endif // OPENZL_HAS_CUDA
