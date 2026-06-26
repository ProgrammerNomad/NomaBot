# Implementation notes

Track divergences between code and architecture docs here. Update formal docs only after Phase 0.1 demo.

## Status

Phase 0.2 implemented (2026-06) - Milestone 2 desktop core.

## What was built (0.2)

- **StateManager** - canonical bot state; services request via EventBus
- **TransportManager** - transport factory; DeviceManager is registry-only
- **Priority queue + CommandDispatcher** - Runtime → queue → transport
- **SQLite** - `devices`, `scheduler_jobs`, `settings` at `%APPDATA%/NomaBot/noma.db`
- **System tray**, scheduler (action+parameters), Windows activity service
- **Settings + log viewer** - rotating logs (`runtime`, `activity`, `scheduler`, etc.)
- **Rich hello handshake** - firmware + desktop store capabilities
- **activity_profiles.json** - user-customizable exe → profile mapping
- **AssetRegistry** stub in Runtime (Milestone 3 hook)

## What was built (0.1)

- `sdk/` - Pydantic protocol v1, NomaClient, NomaBot API, MockDevice, CLI, asset compiler v0
- `desktop/` - PySide6 shell, Noma Runtime, DeviceManager, emulator (170×320), serial transport
- `firmware/` - PlatformIO LilyGO T-Display S3, NDJSON protocol, animation engine v0
- `assets/characters/nomabot/` - placeholder pack with personality.yaml
- `profiles/` - device profile JSON
- CI - GitHub Actions (pytest, ruff, protocol lint, firmware build)

## Drift log

- Python 3.14 used locally if 3.13 unavailable (requires-python >=3.13)
- Firmware renderer uses inline LovyanGFX pin config for T-Display S3
- Character pack loading on device: firmware uses built-in animation names; full manifest load is Milestone 3
- Desktop uses background asyncio thread + Qt main thread; StateManager schedules via `run_coroutine_threadsafe`
- T-Display S3 display uses 8-bit parallel LovyanGFX bus (not SPI)
