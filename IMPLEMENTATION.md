# Implementation notes

Track divergences between code and architecture docs here. Update formal docs only after Phase 0.1 demo.

## Status

Phase 0.1 implemented (2025-06).

## What was built

- `sdk/` — Pydantic protocol v1, NomaClient, NomaBot API, MockDevice, CLI, asset compiler v0
- `desktop/` — PySide6 shell, Noma Runtime, DeviceManager, emulator (170×320), serial transport
- `firmware/` — PlatformIO LilyGO T-Display S3, NDJSON protocol, animation engine v0
- `assets/characters/nomabot/` — placeholder pack with personality.yaml
- `profiles/` — device profile JSON
- CI — GitHub Actions (pytest, ruff, protocol lint, firmware build)

## Drift log

- Python 3.14 used locally if 3.13 unavailable (requires-python >=3.13)
- Firmware renderer uses inline LovyanGFX pin config for T-Display S3
- Character pack loading on device: firmware uses built-in animation names; full manifest load is Phase 0.2
