#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

g++ -std=gnu++20 -O2 \
  contrib/rinhash_trace.cpp \
  src/bitcoin/crypto/blake3/blake3.c \
  src/bitcoin/crypto/blake3/blake3_dispatch.c \
  src/bitcoin/crypto/blake3/blake3_portable.c \
  -DBLAKE3_NO_SSE2 -DBLAKE3_NO_SSE41 -DBLAKE3_NO_AVX2 -DBLAKE3_NO_AVX512 \
  -largon2 \
  -o contrib/rinhash_trace

echo "Built contrib/rinhash_trace"
