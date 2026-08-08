#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENZL_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

CLANGXX="${CLANGXX:-clang++}"
WORKERS="${WORKERS:-4}"
JOBS="${JOBS:-4}"
TIME="${TIME:-10}"
OUT_BIN="/tmp/fuzz_linear_predict"
CORPUS_DIR="/tmp/fuzz_linear_predict_corpus"

mkdir -p "$CORPUS_DIR"

echo "=== Compiling OpenZL linear_predict LibFuzzer target ==="
"$CLANGXX" -O2 -g -fsanitize=fuzzer,address,undefined -march=native \
  -I"$OPENZL_ROOT/include" \
  -I"$OPENZL_ROOT/src" \
  "$SCRIPT_DIR/fuzz_linear_predict.cpp" \
  "$OPENZL_ROOT/src/openzl/codecs/linear_predict/encode_linear_predict_kernel.c" \
  "$OPENZL_ROOT/src/openzl/codecs/linear_predict/decode_linear_predict_kernel.c" \
  -o "$OUT_BIN"

echo "=== Running linear_predict libFuzzer ($WORKERS workers, $TIME sec) ==="
"$OUT_BIN" -workers="$WORKERS" -jobs="$JOBS" -max_total_time="$TIME" "$CORPUS_DIR"
