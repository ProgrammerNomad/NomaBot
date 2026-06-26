# ADR 0005: Event priority and render queue

**Status:** Accepted  
**Date:** 2025-06 (planning)

## Context

Multiple subsystems emit render-related events at once: Spotify track change, git build failure, calendar meeting, battery low (future), user idle. Without priority rules, last-writer-wins feels random and breaks user trust ([UX](../15_UX.md)).

## Decision

All render intents carry a **priority** enum processed by Noma Runtime render queue:

| Priority | Value | Examples |
|----------|-------|----------|
| `CRITICAL` | 4 | Device error, security alert |
| `HIGH` | 3 | Meeting started, build failed |
| `NORMAL` | 2 | Scheduler hydration, git branch change |
| `LOW` | 1 | Spotify now playing |
| `BACKGROUND` | 0 | Ambient weather effect |

Rules:

1. Higher priority wins for conflicting layers (message, accessory, animation)
2. Same priority: merge non-conflicting layers; tie-break by timestamp
3. `BACKGROUND` suppressed during user "focus" or meeting state
4. Queue coalesces duplicate LOW events within 2 s window

## Consequences

**Positive**

- Predictable behavior under load
- Plugins declare default priority in manifest; override per request
- Testable with fixture event bursts

**Negative**

- Authors must choose sensible priorities-document in Plugin SDK

## References

- [Architecture - Event priority](../01_ARCHITECTURE.md#event-priority-and-render-queue)
- [Plugin System](../07_PLUGIN_SYSTEM.md)
- [UX - Work session](../15_UX.md)
