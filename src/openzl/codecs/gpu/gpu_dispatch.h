// Copyright (c) Meta Platforms, Inc. and affiliates.
// Shared plumbing for dispatching codec kernels to the GPU.
//
// Every helper here is written so that a GPU dispatch can be *abandoned* at any
// point: the caller is always able to fall back to the CPU kernel, which is the
// source of truth. A GPU result is only ever published once every CUDA call
// involved in producing it has been checked.

#ifndef ZL_CODECS_GPU_DISPATCH_H
#define ZL_CODECS_GPU_DISPATCH_H

#if defined(OPENZL_HAS_CUDA) && OPENZL_HAS_CUDA

#    include <cuda_runtime.h>
#    include <stdbool.h>
#    include <stddef.h>

#    ifdef __cplusplus
extern "C" {
#    endif

/// Payloads smaller than this stay on the CPU: the PCIe round-trip dominates
/// and the AVX2 kernels are already memory-bound.
#    define ZL_GPU_MIN_NB_ELTS 65536

/// True when OPENZL_USE_GPU names a positive integer *and* no unrecoverable
/// CUDA failure has been seen yet. Evaluated once per process, thread-safely.
bool ZL_GPU_enabled(void);

/// A non-blocking CUDA stream owned by the calling thread. Destroyed when the
/// thread exits, so a pool that churns threads does not leak streams.
/// Returns NULL if no stream could be created; callers must then stay on CPU.
cudaStream_t ZL_GPU_stream(void);

/// Records a failed GPU dispatch. Clears the calling thread's pending CUDA
/// error so a failure here cannot poison unrelated CUDA calls later, logs when
/// OPENZL_GPU_DEBUG is set, and latches GPU dispatch off process-wide for
/// errors that are not plain out-of-memory (those are treated as transient).
/// Always returns false, so it can be used as `return ZL_GPU_failed(...)`.
bool ZL_GPU_failed(cudaError_t err, char const* what);

/// Records a GPU dispatch that produced a usable result. Only used to report
/// dispatch counts under OPENZL_GPU_DEBUG. Always returns true, so it can be
/// used as `return ZL_GPU_succeeded();`.
bool ZL_GPU_succeeded(void);

#    ifdef __cplusplus
}
#    endif

#else // !OPENZL_HAS_CUDA

/// Keeps translation units that include this header non-empty when CUDA
/// support is compiled out.
typedef int ZL_GPU_dispatchDisabled;

#endif // OPENZL_HAS_CUDA

#endif // ZL_CODECS_GPU_DISPATCH_H
