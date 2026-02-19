# Security Policy

## Supported Versions

| Version | Supported |
|---------|-----------|
| 2.1.x   | ✅ Current |
| < 2.1   | ❌ Not supported |

## Scope

This policy covers **Fulcrum-RIN**, the Rincoin-enabled fork of the
[Fulcrum](https://github.com/cculianu/Fulcrum) Electrum-protocol server.
Vulnerabilities in the upstream Fulcrum codebase that do **not** affect the
Rincoin-specific additions should be reported upstream to
[cculianu/Fulcrum](https://github.com/cculianu/Fulcrum/security) first.

## Reporting a Vulnerability

**Please do not open a public GitHub issue for security vulnerabilities.**

Report privately by opening a
[GitHub Security Advisory](https://github.com/takologi/Fulcrum-rin/security/advisories/new)
on this repository.  You will receive an acknowledgement within **5 business
days** and a resolution timeline within **14 business days**.

Include as much of the following as possible:

- Affected version(s)
- A description of the vulnerability and its potential impact
- Steps to reproduce or a proof-of-concept
- Suggested fix (optional but appreciated)

## Security-Sensitive Areas

The following subsystems handle untrusted data from the network and have
heightened review requirements:

| Subsystem | File(s) | Notes |
|-----------|---------|-------|
| Electrum RPC parsing | `src/RPC.cpp`, `src/Servers.cpp` | JSON-RPC from arbitrary clients |
| Bitcoin node RPC client | `src/BitcoinD.cpp` | Responses from `rincoind` |
| Block / tx deserialisation | `src/BTC.cpp`, `src/bitcoin/*.cpp` | Full consensus parsing |
| RinHash (PoW) | `src/bitcoin/crypto/rinhash.cpp` | Argon2d + BLAKE3 + SHA3-256 |
| Storage / RocksDB layer | `src/Storage.cpp` | DB key construction |
| TLS certificate handling | `src/SSLCertMonitor.cpp` | Cert auto-reload |
| Admin RPC (optional) | `src/Servers.cpp` | Localhost-only by default |

## Known Limitations

- The **admin RPC** port (if enabled via `admin =` in `fulcrum.conf`) is
  **not** authenticated.  Bind it to `127.0.0.1` only and protect it with
  firewall rules.
- TLS certificates are reloaded without restarting; ensure private key file
  permissions are `0600` and owned by the service user.
- RocksDB is linked as a static library (`staticlibs/rocksdb`).  If a
  critical RocksDB CVE is disclosed, rebuild with an updated static library.

## Dependency Security Contacts

| Dependency | Channel |
|------------|---------|
| RocksDB | https://github.com/facebook/rocksdb/security |
| libargon2 | https://github.com/P-H-C/phc-winner-argon2/issues |
| BLAKE3 | https://github.com/BLAKE3-team/BLAKE3/security |
| libzmq | https://github.com/zeromq/libzmq/security |
| Qt | https://www.qt.io/blog/tag/security |
| secp256k1 | https://github.com/bitcoin-core/secp256k1/security |
