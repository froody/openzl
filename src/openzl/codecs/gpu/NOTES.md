# GPU acceleration: audit findings and next steps

Audit of the CUDA delta/zigzag offload (commits `aeeef0e`..`7c23f6a`), prompted
by `zli train` failing with `Non-deterministic compression!` whenever
`OPENZL_USE_GPU=1` was set.

Scope: `src/openzl/codecs/gpu/`, plus the dispatch sites in the delta and
zigzag codecs. `contrib/gpu/` and `contrib/pivco-huffman/gpu/` are upstream
OpenZL code, are not part of this build, and were not touched.

## Reproduction

`zli train` on the `points` channel failed within seconds with
`OPENZL_USE_GPU=1` and completed normally with `OPENZL_USE_GPU=0`, on identical
inputs. So the fault was in the GPU path, not in training itself.

## Findings

Ordered by severity. Every one of these is fixed.

1. **All CUDA error codes after the allocations were discarded.** The kernel
   launch, both `cudaMemcpyAsync` calls and `cudaStreamSynchronize` all had
   their return values thrown away. Any failure therefore published an
   *uninitialised* output buffer as if it were valid codec output — which is
   exactly the shape of fault that makes compressed output vary between runs.

2. **There was no fallback path.** The GPU block returned only on success; on
   failure it fell through and committed whatever happened to be in the output
   buffer, rather than re-running the CPU kernel.

3. **Device memory leaked on partial allocation failure.** If the second or
   third `cudaMallocAsync` failed, the buffers already allocated were never
   freed, so the pool grew monotonically for the life of the process.

4. **GPU dispatch lived inside the leaf kernel `ZS_deltaEncode8`.** That file
   documents itself as deliberately free of such dependencies, and the function
   is called directly by the fuzz target and the kernel unit tests — which
   silently became GPU-dependent and environment-dependent. Dispatch now lives
   in `encode_delta_binding.c`, next to the equivalent zigzag dispatch.

5. **Data race on the `OPENZL_USE_GPU` cache.** `static int gpu_enabled = -1`
   was lazily initialised without synchronisation while 64 training threads
   read and wrote it. Now `pthread_once`.

6. **Per-thread CUDA streams were never destroyed.** `static _Thread_local
   cudaStream_t` leaks one stream for every thread that ever runs a codec. Now
   held in a `pthread_key` with a destructor.

7. **32-bit index arithmetic in the kernels.** `blockIdx.x * blockDim.x` was
   evaluated in `unsigned int` before being widened, and the block count was
   cast to `int`. Both now compute in `size_t`, and the kernels use grid-stride
   loops so a capped grid still covers the whole payload.

8. **Missing `default:` in the zigzag width switch.** The preceding
   `ZL_ASSERT` compiles away under `NDEBUG`, and the release build is
   `-O3 -DNDEBUG`, so an unexpected element width would have launched no kernel
   at all and then copied uninitialised device memory into the output.

9. **Thrust exceptions could escape an `extern "C"` frame.**
   `thrust::inclusive_scan` throws when its temporary allocation fails, which
   would have reached C code and called `std::terminate`. Now caught and
   reported as an ordinary CUDA error.

10. **Built for `sm_75` on an `sm_86` card.** It worked via PTX JIT, but paid a
    JIT compile on every process start. `CMAKE_CUDA_ARCHITECTURES` now defaults
    to `native` (CMake >= 3.24).

Not a defect, but fragile: `getenv`/`atoi` were used with no `<stdlib.h>`; it
only compiled because `cuda_runtime.h` happens to pull it in.

### What was *not* established

Which single defect produced the corruption was not isolated — the fix closes
all of them together. Worth noting that post-fix runs report zero CPU
fallbacks in the single-threaded case, i.e. no CUDA call is actually failing
there. So either the culprit is one of the non-error defects (5 or 7), or it
only surfaces under the 64-thread training load. If this ever recurs, bisecting
the individual defects is the way to settle it.

## Design after the fix

`gpu_dispatch.{h,c}` holds the shared plumbing. The invariant it exists to
enforce is: **a GPU result is only published once every CUDA call that produced
it has been checked.** Any failure abandons the dispatch and the caller runs
the CPU kernel, which is the source of truth.

- `ZL_GPU_enabled()` — `OPENZL_USE_GPU`, read once via `pthread_once`.
- `ZL_GPU_stream()` — per-thread non-blocking stream, destroyed at thread exit.
- `ZL_GPU_failed()` — clears the thread's pending CUDA error so a failed
  dispatch cannot poison later CUDA calls, and latches GPU dispatch off
  process-wide for anything that is not a plain out-of-memory (a dead context
  or a missing kernel image will not fix itself; OOM under load might).
- `OPENZL_GPU_DEBUG=1` logs fallbacks and prints dispatch/fallback counts at
  exit. Useful for confirming the GPU path is actually being taken.

## Verification

- **Bit-exactness:** 60 samples, fixed compressor, GPU output byte-identical to
  CPU output, all frames round-tripping back to the original. 458 GPU
  dispatches with 0 fallbacks, so the GPU path was genuinely exercised.
- **`tests/unittest/transforms/test_gpu_kernels.cpp`:** table-driven GPU-vs-CPU
  equivalence over every width, at sizes straddling the block size, the offload
  threshold, and the capped grid. Mutation-tested: a correct-but-differently-
  shaped kernel passes, a broken grid stride fails all three tests.
- **`tests/gpu/run_test_gpu.sh`** still passes with the rewritten kernels.

## The bigger problem: this offload is a net loss

Fixed compressor, 9 MB `points` sample, single-threaded:

| | time | throughput |
|---|---|---|
| `OPENZL_USE_GPU=0` | ~178 ms | ~51 MB/s |
| `OPENZL_USE_GPU=1` | ~340 ms | ~26 MB/s |

The kernels are not the problem — they run at ~200 GB/s. The problem is the
shape of the offload. Delta and zigzag are roughly one operation per byte, so
they are bandwidth-bound, and the current design pays a full
`cudaMallocAsync` + H2D + kernel + D2H + `cudaStreamSynchronize` per codec
invocation on payloads of a few MB. Moving the bytes across PCIe costs more
than the CPU spends doing the entire job with AVX2.

The 38-60x figure printed by `tests/gpu/run_test_gpu.sh` is device-resident
kernel time only; it excludes every transfer.

**So: correct now, but leave `OPENZL_USE_GPU` unset for production runs.**

## Next steps

Roughly in order of value.

1. **Decide whether to keep this at all.** As structured, per-codec offload of
   bandwidth-bound kernels cannot win. If the goal was faster training, the
   answer is probably elsewhere (more CPU threads, cheaper candidate
   evaluation) rather than in these two codecs.

2. **If the GPU is worth pursuing, move the boundary, not the kernel.** The
   only way this pays is keeping data resident on the device across a whole
   graph — dispatch a subgraph (e.g. delta -> zigzag -> entropy stage) with one
   H2D at the start and one D2H at the end, instead of a round trip per codec.
   That is a substantially different design from what exists here.

3. **Cheaper intermediate win: stop allocating per call.** A per-thread
   arena reused across dispatches, plus pinned staging buffers for the host
   side, would remove the allocation and let the copies actually overlap.
   Worth measuring before committing to (2); it will narrow the gap but
   probably not close it.

4. **Raise `ZL_GPU_MIN_NB_ELTS`.** 65536 elements is far below break-even.
   Measure where the crossover actually is and set the threshold from data —
   on this hardware it may be that no size is a win, which is itself the answer
   to (1).

5. **Wire the decode kernels up or delete them.** `ZS_deltaDecode*_gpu` and
   `ZL_zigzagDecode*_gpu` are built and tested but nothing in the library calls
   them. Dead code that looks live is a trap for the next reader.

6. **Get the tests into CI.** `tests/unittest/transforms/test_gpu_kernels.cpp`
   only builds with `OPENZL_BUILD_TESTS=ON` *and* `OPENZL_ENABLE_CUDA=ON`;
   nothing currently runs that combination. Until it does, these kernels are
   unguarded against regression.

7. **Add a multithreaded soak test.** Every fault here was load- and
   timing-dependent. A test that hammers the dispatch path from N threads and
   diffs against the CPU kernel would have caught this class of bug directly.
