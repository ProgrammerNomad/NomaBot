# ADR 0002: JSON protocol with version field

**Status:** Accepted  
**Date:** 2025-06 (planning)

## Context

Desktop and firmware communicate over Serial, WebSocket, MQTT, and future transports. Messages must be debuggable, extensible, and versionable across independent release cycles.

Binary protocols are smaller but harder to inspect; plain text without schema invites drift.

## Decision

Use **newline-delimited JSON (NDJSON)** on serial and JSON objects on framed transports. Every message includes **`"v": <integer>`** protocol version. Commands use `{ v, id, type, cmd, params }`.

Negotiate max supported version in `hello` / `hello_ack`.

## Consequences

**Positive**

- Human-readable logs in `transport.log`
- Same parser for all transports
- Optional fields and new commands without breaking old clients when `v` unchanged

**Negative**

- Higher byte overhead than binary (acceptable at desk scale)
- Requires strict max-line limits and fuzz testing

**Mitigation**

- Hot-path compact keys deferred until profiling proves need
- Host-side parser fuzz in CI ([Testing](../14_TESTING.md))

## References

- [Communication](../04_COMMUNICATION.md)
- [Security — protocol hardening](../17_SECURITY.md)
