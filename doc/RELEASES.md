# Fulcrum-RIN Release Notes

Release notes for the Rincoin fork of [cculianu/Fulcrum](https://github.com/cculianu/Fulcrum).

Version scheme: `<upstream-baseline>-rin.<N>`, where `<upstream-baseline>` is the
upstream Fulcrum version this build is rebased on, and `<N>` is the fork-release
counter. The counter resets to `1` on every upstream baseline bump.

The banner reported via Electrum `server.version` is:

```
Fulcrum-RIN <fork-version> (Fulcrum <upstream-version>)
```

The upstream baseline component is exposed deliberately so that downstream
wallets and security advisories can map any upstream CVE directly to the
corresponding Fulcrum-RIN build.

---

## 2.1.1-rin.1 — 2026-05-25

Upstream baseline: Fulcrum **2.1.1** (commit `4b38a75`, tag `v2.1.1`).

Pure upstream-sync release. Pulled 10 commits from `cculianu/Fulcrum`
between `8d8453d` (the previous baseline) and `4b38a75`. No fork-side
code changes; only documentation, build-script and config additions on
top of the merge.

### Upstream changes pulled

- Bumped to upstream tag **v2.1.1** (`f06d28f`).
- BCHN sync for `bitcoin/token.h`; token-test extra check and minor C++
  nits in `Mempool.cpp`, `BlockProc.cpp`, and `Storage/Compat.cpp`
  (no behaviour change).
- Windows build script: `wine64` → `wine`.
- `electrum-cash-protocol` submodule bump.

See `git log v2.1.0-rin.2..HEAD` for the full upstream commit list.

### Fork-side documentation

- README: new **Runtime dependencies** section with the exact `apt-get`
  command (`libqt5network5 libqt5core5a libzmq5 libbz2-1.0 zlib1g`) and
  Fedora/RHEL equivalent. Build-time deps updated to include
  `libargon2-dev`. Library versions in the README now reflect the
  release build matrix (Qt 5.15.8, gcc 12.2.0, etc.).
- `doc/fulcrum-rin.conf`: top comment block points operators at the
  same `apt-get` line.
- `resources/rin/servers.json`: switched seed peers to
  `peer{1,2,3}.rincoin.tech`, protocol version `1.4.2`.

### Compatibility

- Drop-in replacement for `2.1.0-rin.2`. No on-disk format change.
- Electrum-protocol-visible banner is now
  `Fulcrum-RIN 2.1.1-rin.1 (Fulcrum 2.1.1)`.

---

## 2.1.0-rin.2 — 2026-05-01

Upstream baseline: Fulcrum **2.1.0** (commit `8d8453d`).

Hardening, documentation, and operational-safety release. No consensus
changes; no on-disk database format changes; safe drop-in upgrade from
`2.1.0-rin.1`.

### Security & robustness

- **RinHash salt:** replaced the `reinterpret_cast<uint8_t*>(const_cast<char*>(...))`
  on a string literal in `src/bitcoin/crypto/rinhash.cpp` with a properly typed
  `static constexpr uint8_t salt_bytes[]`. Eliminates undefined behaviour on
  any toolchain that places string literals in read-only memory.
- **PoW exception safety:** `CBlockHeader::GetHash()` now wraps the `RinHash()`
  call in `try/catch`, returning `uint256()` on libargon2 failure instead of
  letting the exception propagate up the validation path and crash the server.
- **Banner reveals upstream baseline:** `server.version` now reports
  `Fulcrum-RIN <fork-version> (Fulcrum <upstream-version>)`, enabling CVE
  triage against upstream advisories.

### Operations & defaults

- **`doc/fulcrum-rin.conf` rewritten** as a "private back-end safe" template:
  - All listening sockets bind to `127.0.0.1` by default; plaintext `tcp` is
    commented out.
  - `peering = false` shipped explicitly, with the rationale documented in-line
    and a "PUBLIC NODE" callout describing the switches needed for a
    community-serving node.
  - Cookie-file authentication is presented as the recommended option ahead of
    static `rpcuser`/`rpcpassword`.
  - Admin RPC section carries an unmissable "NO AUTHENTICATION" warning.
  - `max_clients` vs upstream PeerMgr connection cap clarified (with reference
    to upstream commit `acd2685`).

### Documentation

- **SLIP-0044 status finalized.** [`doc/electrum-rin-fork-manual.md`](electrum-rin-fork-manual.md)
  records that coin type **`9555`** (`0x80002553`) for `RIN` / Rincoin is now
  officially registered in
  [satoshilabs/slips slip-0044.md](https://github.com/satoshilabs/slips/blob/master/slip-0044.md).
  The originally chosen placeholder value matches the registered value, so
  derivation paths under `m/44'/9555'/...` and `m/84'/9555'/...` are permanent
  and no wallet migration is required.
- **`UPSTREAM_VERSION` macro** added to `src/Common.h`, with a comment
  describing the per-merge update procedure.
- **TODO marker** added to `BTC.cpp::testRinHash` calling out the absence of
  intermediate-block regression vectors.

### Build & tooling

- `contrib/build-rinhash-trace.sh` now writes its output to `dist/rinhash_trace`
  and ships with a debug-only-tool warning header.
- `.gitignore` updated to cover `fulcrum-rin`, `/dist/`, `/build/`, and
  `contrib/rinhash_trace`.

### Verification

- `qmake && make -j4` builds cleanly on Linux with Qt 5.15.
- `./fulcrum-rin --test rinhash` passes (block 0 and block 1 vectors).

---

## 2.1.0-rin.1 — initial Rincoin port

Upstream baseline: Fulcrum **2.1.0**.

First Rincoin-capable release of the fork. This release was not formally
tagged at the time and is recorded retroactively for completeness; the
2.1.0-rin.2 build is the first with an explicit version string carrying the
`-rin.N` suffix.

### Coin support

- **`coin = RIN`** added as a recognised coin type, gating all
  Rincoin-specific code paths.
- **RinHash proof-of-work verification** integrated:
  BLAKE3 → Argon2d (`t = 2`, `m = 64 KiB`, `lanes = 1`,
  `salt = "RinCoinSalt"`, `v = ARGON2_VERSION_13`) → SHA3-256.
  Implemented in `src/bitcoin/crypto/rinhash.cpp` and consumed from
  `CBlockHeader::GetHash()` when `coin == RIN`.
- **`estimatesmartfee` enabled** for `isRIN`, mirroring Bitcoin Core
  semantics so wallets get sane fee estimates.
- **Bech32 HRP** `rin` (mainnet) / `trin` (testnet) / `rrin` (regtest)
  recognised by address parsing and scripthash derivation.

### Network & operations

- **Rincoin peer-discovery seed servers** added to the bootstrap list.
- **WSS allowed by default** in the shipped `fulcrum-rin.conf` template,
  enabling browser-based Electrum clients out of the box.
- **`fulcrum-rin` binary** name distinguishes the build from upstream
  installs that may coexist on the same host.

### Tooling

- `contrib/build-rinhash-trace.sh` introduced to produce a standalone
  RinHash trace binary for cross-implementation verification against
  Rincoin Core, cpuminer-opt-rin, and the wallet implementations.
- `--test rinhash` self-test added covering Rincoin mainnet block 0 and
  block 1 PoW hashes.

### Branding

- Admin banner and startup log identify the build as **Fulcrum-RIN** while
  leaving `APPNAME = "Fulcrum"` so the Electrum wire protocol remains
  byte-identical to upstream.
