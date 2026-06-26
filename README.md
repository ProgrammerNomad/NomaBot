# NomaBot

A **desk companion platform**: PySide6 desktop control plane, ESP32 render devices, compiled character packs, plugins, and SDKs-built for an open ecosystem from day one.

```
Plugins / Services  →  Noma Runtime  →  Transport  →  ESP32 (170×320)
                              ↑
                       Asset Compiler
```

**Status:** Phase 0.1 implemented - see [IMPLEMENTATION.md](IMPLEMENTATION.md) and [docs/18_BUILD.md](docs/18_BUILD.md). **Hardware:** [LILYGO T-Display S3](docs/09_HARDWARE.md).

## Quick start

```bash
uv sync --all-packages
uv run python scripts/generate_placeholder_sprites.py
just test
just emulator
```

USB: `uv run python -m nomabot_desktop --port COM7`

## What it is

NomaBot reacts to your workflow-coding, music, git status, scheduled reminders-with layered sprite characters on a desk display. The ESP32 **only renders**. **Noma Runtime** on the desktop orchestrates scheduler, priority render queue, and device routing.

## Documentation

### Core

| Document | Description |
|----------|-------------|
| [**Vision**](docs/00_VISION.md) | Platform principles |
| [**Architecture**](docs/01_ARCHITECTURE.md) | Noma Runtime, priority queue, multi-repo |
| [**Roadmap**](docs/10_ROADMAP.md) | Milestones + implementation order |
| [**Contributing**](docs/CONTRIBUTING.md) | How to participate |
| [**ADR**](docs/adr/README.md) | Architecture decision records |

### Engineering

| Document | Description |
|----------|-------------|
| [Desktop App](docs/02_DESKTOP_APP.md) | PySide6, scheduler, logging |
| [Firmware](docs/03_FIRMWARE.md) | Animation graph, renderer abstraction |
| [Communication](docs/04_COMMUNICATION.md) | Protocol `v`, streaming |
| [Animation Engine](docs/05_ANIMATION_ENGINE.md) | Layers + graph |
| [Character System](docs/06_CHARACTER_SYSTEM.md) | Packs + **personality** |
| [Plugin System](docs/07_PLUGIN_SYSTEM.md) | Permissions |
| [AI](docs/08_AI.md) | Providers + personality prompts |
| [Hardware](docs/09_HARDWARE.md) | **LILYGO T-Display S3** + experimental boards |
| [Asset Pipeline](docs/11_ASSET_PIPELINE.md) | Compiler, Character Editor |
| [SDK](docs/12_SDK.md) | `NomaBot.play_animation()` high-level API |
| [Platform](docs/13_PLATFORM.md) | Themes, Theme Editor, telemetry, store |

### Product & quality

| Document | Description |
|----------|-------------|
| [**Testing Strategy**](docs/14_TESTING.md) | CI, mocks, golden renders |
| [**UX**](docs/15_UX.md) | What the user should *feel* |
| [**Offline Mode**](docs/16_OFFLINE.md) | USB-only = complete product |
| [**Security**](docs/17_SECURITY.md) | Pairing, OTA, sandbox |

## Design highlights

- **Noma Runtime** - single orchestrator (not services → device directly)
- **Event priority** - Critical / High / Normal / Low / Background
- **PySide6 from day one** - no UI migration
- **170×320 official profile** - LILYGO T-Display S3
- **Offline-first** - USB-only is a feature, not fallback
- **Telemetry off by default** - opt-in only

## License

License TBD - see `LICENSE` when added.
