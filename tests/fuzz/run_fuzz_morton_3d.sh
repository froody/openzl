#!/usr/bin/env bash
# Coverage-guided LLVM libFuzzer runner for OpenZL Morton 3D spatial curve kernels.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENZL_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

CLANGXX="${CLANGXX:-clang++}"
WORKERS="${WORKERS:-16}"
JOBS="${JOBS:-16}"
TIME="${TIME:-30}"
OUT_BIN="/tmp/fuzz_morton_3d"
CORPUS_DIR="/tmp/fuzz_morton_3d_corpus"

mkdir -p "$CORPUS_DIR"

echo "=== Compiling OpenZL Morton 3D LibFuzzer target ==="
"$CLANGXX" -O2 -g -fsanitize=fuzzer,address,undefined -march=native \
  -I"$OPENZL_ROOT/include" \
  -I"$OPENZL_ROOT/src" \
  "$SCRIPT_DIR/fuzz_morton_3d.cpp" \
  "$OPENZL_ROOT/src/openzl/codecs/morton_3d/encode_morton_3d_kernel.c" \
  "$OPENZL_ROOT/src/openzl/codecs/morton_3d/decode_morton_3d_kernel.c" \
  "$OPENZL_ROOT/build/libopenzl.a" \
  -o "$OUT_BIN"

echo "=== Running Morton 3D libFuzzer ($WORKERS workers, $TIME sec) ==="
"$OUT_BIN" -workers="$WORKERS" -jobs="$JOBS" -max_total_time="$TIME" "$CORPUS_DIR"
