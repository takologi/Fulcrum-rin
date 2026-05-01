#!/usr/bin/env bash
#
# build-rinhash-trace.sh — build the standalone `rinhash_trace` debug tool.
#
# WARNING: the produced binary is for *local debugging only*. It is not
# hardened, has no input sanitization beyond a strict 80-byte hex check,
# and must NEVER be invoked from RPC handlers, web front-ends, or any
# context that takes untrusted input. It is shipped from `contrib/` for
# the same reason upstream Fulcrum ships its admin helpers there.
#
set -euo pipefail

cd "$(dirname "$0")/.."

mkdir -p dist

g++ -std=gnu++20 -O2 \
  contrib/rinhash_trace.cpp \
  src/bitcoin/crypto/blake3/blake3.c \
  src/bitcoin/crypto/blake3/blake3_dispatch.c \
  src/bitcoin/crypto/blake3/blake3_portable.c \
  -DBLAKE3_NO_SSE2 -DBLAKE3_NO_SSE41 -DBLAKE3_NO_AVX2 -DBLAKE3_NO_AVX512 \
  -largon2 \
  -o dist/rinhash_trace

echo "Built dist/rinhash_trace"
