#!/usr/bin/env bash
# Coverage-guided LLVM libFuzzer runner for OpenZL Pcodec kernels.
# Limits workers to 16 cores by default to avoid interfering with training.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENZL_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

CLANGXX="${CLANGXX:-/mnt/data-3/tbirch/home/bin/clang++}"
WORKERS="${WORKERS:-16}"
JOBS="${JOBS:-16}"
TIME="${TIME:-30}"
OUT_BIN="/tmp/fuzz_pcodec"
CORPUS_DIR="/tmp/fuzz_pcodec_corpus"

mkdir -p "$CORPUS_DIR"

echo "=== Compiling OpenZL Pcodec LibFuzzer target with ASan & UBSan ==="
"$CLANGXX" -O2 -g -fsanitize=fuzzer,address,undefined -march=native \
  -I"$OPENZL_ROOT/include" \
  -I"$OPENZL_ROOT/src" \
  -x c++ "$SCRIPT_DIR/fuzz_pcodec.cpp" \
  -x c "$OPENZL_ROOT/src/openzl/codecs/pcodec/encode_pcodec_kernel.c" \
  -x c "$OPENZL_ROOT/src/openzl/codecs/pcodec/decode_pcodec_kernel.c" \
  -o "$OUT_BIN"

echo "=== Running Pcodec libFuzzer ($WORKERS workers, $TIME sec) ==="
"$OUT_BIN" -workers="$WORKERS" -jobs="$JOBS" -max_total_time="$TIME" "$CORPUS_DIR"
