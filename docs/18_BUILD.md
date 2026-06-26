# Build System

> Practical reference for developing NomaBot. Architecture docs live in [01_ARCHITECTURE.md](./01_ARCHITECTURE.md).

## Prerequisites

| Tool | Purpose | Install |
|------|---------|---------|
| **Python 3.13+** | SDK + desktop | python.org or pyenv |
| **uv** | Python deps & runs | `pip install uv` or [astral.sh/uv](https://docs.astral.sh/uv/) |
| **just** | Task runner | [github.com/casey/just](https://github.com/casey/just) |
| **PlatformIO** | Firmware | VS Code extension or `pip install platformio` |
| **Git** | Version control | — |

Windows: use PowerShell or Git Bash for `just` commands from repo root.

## First-time setup

```bash
cd NomaBot
uv sync
just lint
just test
```

## Common commands

| Command | Action |
|---------|--------|
| `just sync` | Install/update Python dependencies |
| `just test` | Run pytest (sdk + desktop) |
| `just lint` | Ruff check + format check |
| `just format` | Auto-format with Ruff |
| `just protocol` | Lint protocol JSON fixtures |
| `just profiles` | Validate `profiles/*.json` |
| `just firmware` | Build firmware for LILYGO T-Display S3 |
| `just desktop` | Launch desktop app |
| `just emulator` | Launch with 170×320 emulator window |
| `just mock` | Run mock-device tests |
| `just ci` | lint + test + protocol + profiles |

## Project layout

```text
sdk/          # nomabot package (protocol, client, CLI)
desktop/      # nomabot_desktop (PySide6 app)
firmware/     # PlatformIO ESP32 project
profiles/     # Device profiles (JSON)
assets/       # Character packs (source)
```

## Python packaging

- **uv workspace** at repo root links `sdk/` and `desktop/`.
- **hatchling** builds both packages.
- **Ruff** is the only linter/formatter (no flake8/black/isort).

```bash
uv run nomabot protocol lint sdk/tests/fixtures/protocol
uv run python -m nomabot_desktop
```

## Firmware

```bash
cd firmware
pio run -e lilygo_tdisplay_s3
pio run -e lilygo_tdisplay_s3 -t upload
pio device monitor -b 115200
```

Official profile: `profiles/lilygo_tdisplay_s3.json` (170×320).

## Versions

All components start at **0.1.0** until 1.0:

| Component | Location |
|-----------|----------|
| Monorepo | `VERSION` |
| SDK | `sdk/pyproject.toml` |
| Desktop | `desktop/pyproject.toml` |
| Firmware | `firmware/VERSION` |
| Assets | `assets/VERSION` |

## Logs (desktop)

```text
%APPDATA%/NomaBot/logs/
├── desktop.log
└── transport.log
```
