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
- BIP44 coin type policy
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
- PASS if baseline tests + Rincoin tests pass in CI on Linux.

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
| BIP44 derivation | **Placeholder `9555` (Rincoin p2p port) used during development — obvious sentinel that flags unregistered status in any wallet dump. MUST be replaced with SLIP-0044 registered value before first public release.** | 2026-02-19 |
| xpub/xprv version bytes | **Reuse standard BTC bytes (0x0488B21E / 0x0488ADE4)** — already set in Rincoin Core chainparams | 2026-02-19 |

### BIP44 migration warning

**Code reference:** `electrum/constants.py` — `AbstractNet.BIP44_COIN_TYPE` declaration and the `TODO(rincoin-phase1)` stub block for `RincoinMainnet`.

The placeholder value `9555` (Rincoin's p2p port) is intentionally conspicuous so that any wallet
file, seed dump, or log that shows the derivation path immediately reveals it is pre-registration:
`m/44'/9555'/...` and `m/84'/9555'/...` are not used by any real SLIP-0044 entry, guaranteeing no
overlap with existing coins during development.

After SLIP-0044 registration:

1. Replace every occurrence of `9555` in `constants.py` with the registered number.
2. Add a one-time seed migration helper if wallets were distributed with the placeholder (unlikely if done before first release).
3. Record the registered value in this manual and in `electrum/constants.py`.

## 7) Open questions log (still to resolve)

1. Official server seed list and operator trust model
2. Hardware-wallet support in phase 1: include or defer
3. Rincoin SLIP-0044 coin type number (pending registration)

## 8) Change log for this manual

- 2026-02-19: Initial draft created from upstream research and Rincoin chainparams extraction.
- 2026-02-19: Confirmed project decisions: name, platforms, ports, BIP44 policy.
- 2026-02-19: Updated BIP44 placeholder from LTC coin type `2` to sentinel `9555`; added code reference.
- 2026-02-19: **Phase 0 gate PASSED** — upstream Electrum 4.7.0 installed, 301 core tests pass on `rincoin-bootstrap` branch.
- 2026-02-19: **Phase 1 gate PASSED** (`10375421e`) — `RincoinMainnet/Testnet/Regtest` classes + chains/ files committed. Genesis hash `000096bd…` confirmed via Fulcrum `server.features`. 338 tests pass.
- 2026-02-19: **Phase 2 gate PASSED** (`5e3a122`) — 35 address/script/scripthash test vectors all green. P2PKH="R", P2SH="r", bech32="rin1", WIF=0xbc, scripthash matches Fulcrum convention, cross-network isolation verified.
