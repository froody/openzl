#!/usr/bin/env bash
# Build and run OpenZL GPU CUDA kernels on NVIDIA RTX 3090.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENZL_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

NVCC="${NVCC:-/usr/local/cuda-13.1/bin/nvcc}"
if [ ! -x "$NVCC" ]; then
  NVCC="$(which nvcc || echo "nvcc")"
fi

CLANGXX="${CLANGXX:-/mnt/data-3/tbirch/home/bin/clang++}"
if [ ! -x "$CLANGXX" ]; then
  CLANGXX="$(which clang++ || echo "g++")"
fi

OUT_BIN="/tmp/test_gpu_kernels"

echo "=== Compiling OpenZL CUDA GPU Kernels for SM_86 (RTX 3090) ==="
"$NVCC" -O3 -std=c++17 -arch=sm_86 -Xcompiler "-O3 -march=native" \
  -I"$OPENZL_ROOT/include" \
  -I"$OPENZL_ROOT/src" \
  "$SCRIPT_DIR/test_gpu_kernels.cu" \
  "$OPENZL_ROOT/src/openzl/codecs/gpu/encode_delta_gpu.cu" \
  "$OPENZL_ROOT/src/openzl/codecs/gpu/encode_zigzag_gpu.cu" \
  "$OPENZL_ROOT/src/openzl/codecs/delta/encode_delta_kernel.c" \
  "$OPENZL_ROOT/src/openzl/codecs/delta/decode_delta_kernel.c" \
  "$OPENZL_ROOT/src/openzl/codecs/zigzag/encode_zigzag_kernel.c" \
  "$OPENZL_ROOT/src/openzl/codecs/zigzag/decode_zigzag_kernel.c" \
  -o "$OUT_BIN"

echo "=== Executing GPU Kernel Test Suite & Benchmark on NVIDIA GeForce RTX 3090 ==="
"$OUT_BIN"
