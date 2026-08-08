#!/usr/bin/env bash
# Coverage-guided LLVM libFuzzer runner for SIMD Delta & Zigzag kernels.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENZL_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

CLANGXX="${CLANGXX:-/mnt/data-3/tbirch/home/bin/clang++}"
if [ ! -x "$CLANGXX" ]; then
  CLANGXX="$(which clang++ || echo "clang++")"
fi

WORKERS="${WORKERS:-8}"
JOBS="${JOBS:-8}"
TIME="${TIME:-30}"
OUT_BIN="/tmp/fuzz_delta_zigzag"
CORPUS_DIR="/tmp/fuzz_delta_zigzag_corpus"

mkdir -p "$CORPUS_DIR"

echo "=== Compiling OpenZL Delta & Zigzag LibFuzzer target with ASan & UBSan & AVX2 ==="
"$CLANGXX" -O2 -g -fsanitize=fuzzer,address,undefined -march=native \
  -I"$OPENZL_ROOT/include" \
  -I"$OPENZL_ROOT/src" \
  -x c++ "$SCRIPT_DIR/fuzz_delta_zigzag.cpp" \
  -x c "$OPENZL_ROOT/src/openzl/codecs/delta/encode_delta_kernel.c" \
  -x c "$OPENZL_ROOT/src/openzl/codecs/delta/decode_delta_kernel.c" \
  -x c "$OPENZL_ROOT/src/openzl/codecs/zigzag/encode_zigzag_kernel.c" \
  -x c "$OPENZL_ROOT/src/openzl/codecs/zigzag/decode_zigzag_kernel.c" \
  -o "$OUT_BIN"

echo "=== Running Delta & Zigzag libFuzzer ($WORKERS workers, $TIME sec) ==="
"$OUT_BIN" -workers="$WORKERS" -jobs="$JOBS" -max_total_time="$TIME" "$CORPUS_DIR"
