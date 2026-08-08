#!/usr/bin/env bash
# Coverage-guided LLVM libFuzzer runner for OpenZL Wavelet Transform (IWT) kernels.
# Limits workers to 16 cores by default to avoid interfering with training.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENZL_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

CLANGXX="${CLANGXX:-/mnt/data-3/tbirch/home/bin/clang++}"
WORKERS="${WORKERS:-16}"
JOBS="${JOBS:-16}"
TIME="${TIME:-30}"
OUT_BIN="/tmp/fuzz_iwt"
CORPUS_DIR="/tmp/fuzz_iwt_corpus"

mkdir -p "$CORPUS_DIR"

echo "=== Compiling OpenZL IWT LibFuzzer target ==="
"$CLANGXX" -O2 -g -fsanitize=fuzzer,address,undefined -march=native \
  -I"$OPENZL_ROOT/include" \
  -I"$OPENZL_ROOT/src" \
  "$SCRIPT_DIR/fuzz_iwt.cpp" \
  "$OPENZL_ROOT/src/openzl/codecs/iwt/encode_iwt_kernel.c" \
  "$OPENZL_ROOT/src/openzl/codecs/iwt/decode_iwt_kernel.c" \
  -o "$OUT_BIN"

echo "=== Running IWT libFuzzer ($WORKERS workers, $TIME sec) ==="
"$OUT_BIN" -workers="$WORKERS" -jobs="$JOBS" -max_total_time="$TIME" "$CORPUS_DIR"
