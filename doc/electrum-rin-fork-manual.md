# Rincoin Electrum Fork Manual (Living Document)

Status: Draft v0 (created 2026-02-19)

## 1) Scope and goals

This manual defines how to fork upstream Electrum into a Rincoin wallet that:
- Connects to Rincoin Fulcrum servers (Electrum protocol)
- Uses Rincoin network/address/serialization rules safely
- Maintains high code quality and upstream compatibility discipline
- Ships with reproducible tests and CI checks, including Snyk scans

Non-goals for initial fork:
- Lightning support
- Custom protocol extensions beyond Electrum standard methods
- Mobile-first packaging in phase 1 (desktop/CLI first)

## 2) Upstream baseline and standards

Primary upstream: `spesmilo/electrum` (latest stable tag at fork start).
Reference fork patterns: `pooler/electrum-ltc` (active), historical `electrum-vtc` (useful patterns, outdated).

Standards to follow:
- Keep fork delta minimal and auditable
- Prefer additive or isolated coin-parameter changes
- Preserve upstream tests and style checks
- Rebase/merge upstream regularly (document cadence)
- Security checks mandatory in CI (SAST + dependency scans)

## 3) Rincoin chain constants required for wallet fork

From Rincoin Core chainparams (to be revalidated before coding):
- Mainnet P2PKH prefix: 60 (`R...`)
- Mainnet P2SH prefix: 122 (`r...`)
- Mainnet WIF prefix: 188
- Mainnet Bech32 HRP: `rin`
- Mainnet default p2p port: 9555
- Mainnet message start: `52 49 4E 43`
- Testnet Bech32 HRP: `trin`
- Testnet default p2p port: 19555
- Regtest Bech32 HRP: `rrin`

Electrum-specific values still needed before implementation:
- Electrum server default TCP/SSL ports for Rincoin
- Genesis hash(es) to embed for server validation
- Checkpoint strategy and source (Fulcrum / Core)
- BIP44 coin type: **`9555` (registered SLIP-0044, `0x80002553`)** — settled
- xpub/xprv version policy (reuse or custom)

## 4) Phased execution plan with test gates

### Phase 0 — Workspace setup and baseline freeze
Tasks:
1. Clone upstream Electrum next to Fulcrum-rin.
2. Create branch `rincoin-bootstrap` from chosen upstream tag.
3. Record exact upstream commit and toolchain versions.

Test gate:
- PASS if upstream Electrum installs and runs on this host (`pip install -e .`, unit tests smoke subset pass).

### Phase 1 — Coin parameter integration (minimal functional fork)
Tasks:
1. Duplicate/adapt network definitions for Rincoin mainnet/testnet/regtest.
2. Set prefixes, HRP, genesis, and protocol constants.
3. Wire server list bootstrap (pointing initially to your local Fulcrum).
4. Keep app name/icons unchanged temporarily to reduce risk.

Test gate:
- **PASSED** (`10375421e`) — `RincoinMainnet`/`Testnet`/`Regtest` registered; all params verified against `chainparams.cpp`; servers.json points to `127.0.0.1:50001`; 338 upstream tests pass.

### Phase 2 — Transaction and script correctness validation
Tasks:
1. Validate address parsing/encoding vectors for Base58 + Bech32.
2. Validate script hash derivation compatibility with Fulcrum.
3. Validate tx serialization/signing round-trips against Rincoin Core mempool acceptance.

Test gate:
- **PASSED** (`5e3a122`) — 35 tests / 45 subtests in `tests/test_rincoin_params.py`; coverage: network constants, P2PKH/P2SH/bech32 encoding, WIF round-trip, scripthash derivation, address-to-script structure, cross-network isolation.

### Phase 3 — Full test suite enablement
Tasks:
1. Run upstream unit tests; mark coin-specific expected deltas.
2. Add Rincoin-specific tests (network constants, address vectors, script-hash vectors).
3. Add integration tests against ephemeral/local Fulcrum endpoint.

Test gate:
- **PASSED** (`99b0f249`) — 11 integration tests pass against live Fulcrum-RIN at `127.0.0.1:50001`; skip-on-unreachable guard in place for offline CI. Coverage: genesis hash, header bytes, chain tip height, scripthash P2PKH+P2WPKH accepted, `confirmed` balance zero.

### Phase 4 — Security hardening and Snyk readiness
Tasks:
1. Enable pinned dependencies and lockfile strategy.
2. Add Snyk OSS + Code scans in CI.
3. Add Bandit/pip-audit/semgrep (or equivalent) for layered checks.
4. Remove insecure defaults; enforce TLS guidance for remote servers.

Test gate:
- PASS if CI reports no High/Critical vulnerabilities (or documented, time-bounded exceptions).

### Phase 5 — Branding and release engineering
Tasks:
1. Rename app/package identifiers to Rincoin wallet naming.
2. Replace assets/icons, desktop entry, metadata, and about text.
3. Add deterministic build instructions and release-signing process.

Test gate:
- PASS if packaged app builds, runs, and verifies signatures in a clean environment.

### Phase 6 — Upstream-sync and maintenance policy
Tasks:
1. Define upstream sync cadence (e.g., monthly + critical CVEs).
2. Keep fork delta documented by category (network params, branding, tests).
3. Add contribution rules and security disclosure policy.

Test gate:
- PASS if at least one upstream merge/rebase rehearsal is completed without functional regressions.

## 5) CI/CD baseline (target)

Minimum CI jobs:
- Lint + format checks
- Unit tests
- Integration smoke against local Rincoin Fulcrum
- Dependency audit (`pip-audit`)
- Snyk Open Source and Snyk Code
- Build artifact smoke (run app help/version)

## 6) Project decisions log

| Decision | Choice | Date |
|---|---|---|
| Wallet product name | **Electrum-RIN** | 2026-02-19 |
| Phase 1 platforms | **Linux desktop + Windows desktop** | 2026-02-19 |
| Default Electrum ports | **50001 (TCP) / 50002 (SSL)** | 2026-02-19 |
| BIP44 derivation | **`9555` — registered SLIP-0044 coin type for RIN / Rincoin** (`0x80002553`). The value originally chosen as a placeholder (Rincoin's p2p port) was accepted as the registered number, so no migration is required. | 2026-05-01 |
| xpub/xprv version bytes | **Reuse standard BTC bytes (0x0488B21E / 0x0488ADE4)** — already set in Rincoin Core chainparams | 2026-02-19 |

### BIP44 derivation — registered status

**Status: REGISTERED.** Coin type `9555` is the official SLIP-0044 entry for RIN / Rincoin
(see [satoshilabs/slips slip-0044.md](https://github.com/satoshilabs/slips/blob/master/slip-0044.md),
row `9555 | 0x80002553 | RIN | Rincoin`). The number `9555` was originally selected as a
conspicuous placeholder during early Electrum-RIN development; it was accepted as the
registered value, so all derivation paths under `m/44'/9555'/...` and `m/84'/9555'/...`
are now permanent and standards-compliant. No wallet migration is required.

**Code reference:** `electrum/constants.py` — `AbstractNet.BIP44_COIN_TYPE = 9555` for
`RincoinMainnet`. The historical `TODO(rincoin-phase1)` marker may be removed once the
constant is finalized in code.

`m/44'/9555'/...` (legacy P2PKH) and `m/84'/9555'/...` (native SegWit / bech32) are the
canonical derivation paths for Rincoin. Hardware wallets, third-party explorers, and any
future multi-currency wallet integrating Rincoin should use `9555` (`0x80002553` hardened).

Downstream housekeeping (one-time):

1. Remove the `TODO(rincoin-phase1)` marker around the `BIP44_COIN_TYPE` declaration in `electrum/constants.py`.
2. Reflect the registered status in any release notes that previously warned about the placeholder.
3. No seed-migration helper is needed — the placeholder and the registered value are identical.

## 7) Open questions log (still to resolve)

1. Official server seed list and operator trust model
2. Hardware-wallet support in phase 1: include or defer

## 8) Change log for this manual

- 2026-02-19: Initial draft created from upstream research and Rincoin chainparams extraction.
- 2026-02-19: Confirmed project decisions: name, platforms, ports, BIP44 policy.
- 2026-02-19: Updated BIP44 placeholder from LTC coin type `2` to sentinel `9555`; added code reference.
- 2026-02-19: **Phase 0 gate PASSED** — upstream Electrum 4.7.0 installed, 301 core tests pass on `rincoin-bootstrap` branch.
- 2026-02-19: **Phase 1 gate PASSED** (`10375421e`) — `RincoinMainnet/Testnet/Regtest` classes + chains/ files committed. Genesis hash `000096bd…` confirmed via Fulcrum `server.features`. 338 tests pass.
- 2026-02-19: **Phase 2 gate PASSED** (`5e3a122`) — 35 address/script/scripthash test vectors all green. P2PKH="R", P2SH="r", bech32="rin1", WIF=0xbc, scripthash matches Fulcrum convention, cross-network isolation verified.
- 2026-02-19: **Phase 3 gate PASSED** (`99b0f249`) — 11 Fulcrum integration tests pass; genesis header bytes verified against live server; skip guard for offline CI.
- 2026-05-01: **SLIP-0044 registration confirmed** — coin type `9555` (`0x80002553`) accepted as the official entry for `RIN` / Rincoin in [satoshilabs/slips slip-0044.md](https://github.com/satoshilabs/slips/blob/master/slip-0044.md). The originally chosen placeholder value matches the registered value, so no derivation-path migration is required. BIP44 migration warning removed; `m/44'/9555'/...` and `m/84'/9555'/...` are now permanent.
