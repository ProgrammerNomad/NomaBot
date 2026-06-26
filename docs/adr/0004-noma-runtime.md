# ADR 0004: Noma Runtime as central orchestrator

**Status:** Accepted  
**Date:** 2025-06 (planning)

## Context

Plugins, services, and scheduler jobs all need to affect device output. Letting each service call `DeviceManager.send()` directly creates ordering bugs, duplicate timers, and tight coupling. Users report simultaneous events (Spotify change, build failed, meeting started)—something must arbitrate.

Unreal Engine routes gameplay through a central tick and subsystems; NomaBot needs an equivalent **internal runtime** (called **Noma Runtime**—not an OS).

## Decision

Introduce **Noma Runtime** (`desktop/core/runtime.py`) as the sole path from application logic to device:

```text
Plugins / Services / Scheduler
            │
            ▼
       Event Bus (signals)
            │
            ▼
       Noma Runtime
            │
            ├── Scheduler queue
            ├── Render queue (priority)
            ├── Animation queue (graph events)
            └── DeviceManager → Transport
            │
            ▼
          ESP32
```

Plugins **never** call transport or JSON directly—they emit events or submit `RenderRequest` to runtime.

## Consequences

**Positive**

- Single place for priority, coalescing, offline template fallback
- Plugins become simple event handlers
- Mock runtime in tests replaces entire device stack
- Easier to log every outbound command

**Negative**

- Runtime becomes critical path—requires tests and clear API
- Small indirection latency (negligible vs USB)

## Alternatives considered

- **Services talk to DeviceManager directly** — rejected; priority conflicts inevitable
- **Separate render microservice process** — rejected for v1 complexity

## References

- [Architecture — Noma Runtime](../01_ARCHITECTURE.md#noma-runtime)
- [ADR 0005 — Event priority](./0005-event-priority.md)
- [SDK — NomaBot high-level API](../12_SDK.md#high-level-developer-api)
