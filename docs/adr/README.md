# Architecture Decision Records

This folder records **significant architectural decisions** so future contributors understand *why*, not just *what*.

## Format

Each ADR follows:

- **Status** — Proposed | Accepted | Deprecated | Superseded
- **Context** — Problem and constraints
- **Decision** — What we chose
- **Consequences** — Tradeoffs

## Index

| ADR | Title | Status |
|-----|-------|--------|
| [0001](./0001-use-pyside6.md) | Use PySide6 from day one | Accepted |
| [0002](./0002-json-protocol.md) | JSON protocol with version field | Accepted |
| [0003](./0003-renderer-abstraction.md) | Renderer abstraction in firmware | Accepted |
| [0004](./0004-noma-runtime.md) | Noma Runtime as central orchestrator | Accepted |
| [0005](./0005-event-priority.md) | Event priority and render queue | Accepted |

When a decision changes, add a new ADR that supersedes the old one—do not silently rewrite history.

## Related

- [Architecture](../01_ARCHITECTURE.md)
- [Contributing](../CONTRIBUTING.md)
