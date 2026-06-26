# Security

> **Status:** Design specification — required before Wi-Fi milestone ships.

## Overview

Security spans device pairing, transport authentication, OTA integrity, plugin sandboxing, and asset verification. USB-only MVP has a smaller surface; **Wi-Fi expands risk**—design controls now, implement per milestone.

## Threat model (summary)

| Threat | Mitigation |
|--------|------------|
| Unauthorized device control on LAN | Pairing + session token |
| Malicious JSON to device | Parser limits, command allowlist |
| Tampered firmware OTA | Signed images, rollback |
| Tampered character packs | Manifest hashes, compile-time verify |
| Malicious plugin | Permissions, no raw disk, review for bundled |
| Stolen AI API keys | OS credential store |
| Fake catalog packages | Hash + signature (store phase) |

## Device pairing

### USB (first trust)

```text
1. User plugs device → desktop shows "New device found"
2. User confirms name + "Trust this device"
3. Desktop stores device_id + optional Ed25519 public key from hello_ack
4. Future sessions: match serial + key
```

### Wi-Fi (after Milestone 5)

Pairing requires **physical or USB proof** once:

```text
Option A: USB hello exchanges pairing_secret → Wi-Fi uses derived token
Option B: Display shows 6-digit code → user enters in desktop (short TTL)
```

Unpaired devices reject commands except `hello`, `ping`, `pair_begin`, `pair_confirm`.

### Session authentication

After pairing, commands include:

```json
{
  "v": 1,
  "auth": { "device_id": "…", "token": "…", "seq": 42 }
}
```

Token rotates on reconnect. Firmware rejects replay (`seq` monotonic window).

See [Communication](./04_COMMUNICATION.md) for envelope extension rules (non-breaking optional fields until `v=2` if needed).

## Transport security

| Transport | Default | Hardened |
|-----------|---------|----------|
| Serial | Physical proximity trust | Optional challenge on first plug |
| WebSocket | `ws://` LAN only | `wss://` + TLS cert pin |
| MQTT | Local broker, ACL per device topic | Username/password + TLS |
| TCP/BLE | Disabled until pairing story defined | TBD |

**Never** send API keys, pairing secrets, or tokens in device-bound messages logged to `transport.log`—redact in LoggingService.

## OTA signature

| Requirement | Detail |
|-------------|--------|
| Algorithm | Ed25519 or ECDSA (TBD in ADR) |
| Signing key | Offline maintainer key; CI signs release artifacts |
| Verification | Bootloader / OTA partition checks signature before flash |
| Rollback | Previous slot bootable if new image fails health check |
| Downgrade | Reject older version unless user holds recovery mode |

Desktop `ota_begin` sends only signed bundles from official CI or user-trusted key import.

## Plugin sandbox

| Control | Implementation |
|---------|----------------|
| Permissions | Manifest → runtime gate ([Plugin System](./07_PLUGIN_SYSTEM.md)) |
| Filesystem | Chroot to `%APPDATA%/NomaBot/plugins/{id}/` |
| Network | Opt-in; logged to `plugins.log` |
| Subprocess | Disabled by default |
| Render spam | Rate limit + priority floor |

Third-party plugins: not bundled without review + permission audit.

## Asset verification

| Stage | Check |
|-------|-------|
| **Compile** | `nomabot build-assets` writes SHA-256 per sprite in manifest |
| **Install** | Desktop verifies manifest before activate |
| **Upload** | `sync_chunk` hash per chunk; `sync_verify` whole manifest |
| **Firmware** | Refuses `sync_activate` if hash mismatch |
| **Store (future)** | Catalog entry signed; desktop rejects unknown signer unless opted in |

Compiled packs are **read-only** on device staging until atomic activate.

## Protocol hardening

| Limit | Value (default) |
|-------|-----------------|
| Max NDJSON line | 16 KB |
| Max `sync_chunk` | 64 KB |
| Parse rate | Drop + `error` after burst |
| Command allowlist | Firmware rejects unknown `cmd` |

Fuzzing: host parser fuzz in CI ([Testing](./14_TESTING.md)).

## Permissions (user-facing)

Settings → Plugins → [Plugin] shows:

```text
This plugin requests:
  · Internet — contact Spotify API
  · Notifications — show now playing on device
```

User can revoke by disable—no silent re-enable.

## Telemetry and privacy

Anonymous telemetry is **off by default**, opt-in only. See [Platform — Telemetry](./13_PLATFORM.md#telemetry). No telemetry payload includes activity titles, git diffs, or message text.

## Security milestones

| Milestone | Deliverable |
|-----------|-------------|
| M1 | Parser limits, no secrets in protocol |
| M1.5 | Mock device + fuzz fixtures |
| M2 | USB trust-on-first-connect |
| M3 | Manifest hashes on compile/install |
| M5 | Wi-Fi pairing + session tokens + signed OTA |
| M7 | Store signature verification |

## Incident response

Security issues: **do not** file public GitHub issues for exploitable bugs. Contact maintainers (security policy TBD). Embargo → patch → CVE if warranted.

## Related documentation

- [Communication](./04_COMMUNICATION.md)
- [Firmware — OTA](./03_FIRMWARE.md)
- [Plugin System](./07_PLUGIN_SYSTEM.md)
- [Testing — protocol fuzz](./14_TESTING.md)
- [ADR 0002 — JSON protocol](./adr/0002-json-protocol.md)
