# Contributing to NomaBot

Thank you for considering a contribution. NomaBot is designed as an extensible desk companion platform—contributions to core, firmware, characters, plugins, and documentation are all welcome.

> **Project phase:** Early planning. Implementation is starting; APIs and pack formats may change until milestone 1 exits. Check [Roadmap](./10_ROADMAP.md) before large PRs.

## Code of conduct

Be respectful and constructive. We follow standard open-source norms: welcome newcomers, critique code not people, assume good intent.

## Ways to contribute

| Area | Examples |
|------|----------|
| **Documentation** | Fix typos, clarify architecture, add diagrams |
| **Firmware** | Display driver, animation engine, OTA |
| **Desktop** | PySide6 UI, services, event bus |
| **Characters** | New packs, sprite fixes, validation tools |
| **Plugins** | Git, Spotify, VS Code, Home Assistant, … |
| **Hardware** | PCB improvements, enclosure STLs |
| **Testing** | Protocol fixtures, service unit tests |

Not sure where to start? Look for issues labeled `good first issue` or ask in discussions.

## Before you code

1. Read [Vision](./00_VISION.md) and [Architecture](./01_ARCHITECTURE.md)
2. Match work to a [Roadmap](./10_ROADMAP.md) milestone when possible
3. For new features, open an issue or discussion first if it changes protocol or pack format
4. Keep firmware **dumb**—no Git/Spotify logic on ESP32

## Development setup (target)

Setup instructions will live here as directories land in the repo. Planned layout:

```text
# Desktop (future)
cd desktop
python -m venv .venv
.venv\Scripts\activate        # Windows
pip install -r requirements.txt
python -m app

# Firmware (future)
cd firmware
# Arduino IDE or PlatformIO — see firmware/README when added

# Character tools (future)
cd sdk/character
pip install -e .
noma-character validate ../../assets/characters/nomabot
```

Requirements (expected):

| Component | Version |
|-----------|---------|
| Python | 3.13+ |
| PySide6 | 6.x |
| Arduino ESP32 core | 3.x |
| PlatformIO | optional |

## Project conventions

### Python (desktop + SDK)

- **Formatter:** Ruff or Black (TBD when code lands)
- **Types:** Type hints on public APIs
- **Imports:** Absolute from package root
- **Async:** Prefer async for I/O-bound services; keep UI thread free
- **Tests:** pytest under `desktop/tests/`, `sdk/tests/`

### C++ (firmware)

- Arduino style for sketches; clearer modules in `lib/`
- No dynamic allocation in hot render path where avoidable
- Board-specific pins in `display/board_config.hpp`, not scattered

### JSON schemas

- Protocol, packs, and plugin manifests are versioned
- Breaking changes require bump in `v` or pack `version` and migration note in PR

### Git commits

- Imperative subject line: `Add USB reconnect backoff`
- Body explains **why** when not obvious
- One logical change per commit when practical

## Pull request process

1. Fork and branch from `main` (`feature/…`, `fix/…`, `docs/…`)
2. Update relevant docs in `docs/` with behavior changes
3. Add or update tests when applicable
4. Ensure CI passes (when configured)
5. Open PR with:
   - **Summary** — what and why
   - **Test plan** — steps you ran
   - **Milestone** — e.g. M2 Desktop
6. Address review feedback; maintainers squash-merge unless asked otherwise

### PR size

Prefer small, reviewable PRs. Split large features: protocol first, implementation second.

## Architecture decisions

Significant changes (new transport, pack schema break, plugin permission model) should include:

- Short **Architecture Decision Record** in PR description or `docs/adr/` (if we add that folder)
- Updates to affected engineering docs

## Adding a character pack

1. Copy template from `sdk/character/templates/` (when available) or follow [Character System](./06_CHARACTER_SYSTEM.md)
2. Run validator before submitting
3. License must allow redistribution
4. Include `preview.png` and credit original art

Submit packs to `assets/characters/` for official bundles, or publish externally for community catalogs.

## Adding a plugin

1. Follow [Plugin System](./07_PLUGIN_SYSTEM.md) manifest spec
2. Request minimal permissions
3. Include unit tests with mock event bus
4. Document settings schema and default behavior

Bundled plugins go through stricter review (security, permissions, maintainability).

## Reporting bugs

Include:

- NomaBot version (desktop + firmware)
- Windows version
- Hardware profile
- Steps to reproduce
- Logs from `%APPDATA%/NomaBot/logs/` (when available)
- Expected vs actual behavior

## Security issues

Do **not** open public issues for exploitable vulnerabilities. Email maintainers (contact TBD when project publishes security policy) with details and reproduction.

## Documentation

Engineering docs live in `docs/`:

| Doc | Topic |
|-----|-------|
| [00_VISION](./00_VISION.md) | Product vision |
| [01_ARCHITECTURE](./01_ARCHITECTURE.md) | System design |
| [02_DESKTOP_APP](./02_DESKTOP_APP.md) | Desktop layers |
| [03_FIRMWARE](./03_FIRMWARE.md) | ESP32 firmware |
| [04_COMMUNICATION](./04_COMMUNICATION.md) | JSON protocol |
| [05_ANIMATION_ENGINE](./05_ANIMATION_ENGINE.md) | Sprites and layers |
| [06_CHARACTER_SYSTEM](./06_CHARACTER_SYSTEM.md) | Character packs |
| [07_PLUGIN_SYSTEM](./07_PLUGIN_SYSTEM.md) | Plugins |
| [08_AI](./08_AI.md) | AI providers |
| [09_HARDWARE](./09_HARDWARE.md) | Reference hardware |
| [10_ROADMAP](./10_ROADMAP.md) | Milestones |
| [11_ASSET_PIPELINE](./11_ASSET_PIPELINE.md) | Compiler, Character Editor, streaming |
| [12_SDK](./12_SDK.md) | Python, Character, Plugin, Firmware SDKs |
| [13_PLATFORM](./13_PLATFORM.md) | Themes, store, telemetry |
| [14_TESTING](./14_TESTING.md) | Testing strategy |
| [15_UX](./15_UX.md) | User experience |
| [16_OFFLINE](./16_OFFLINE.md) | Offline mode |
| [17_SECURITY](./17_SECURITY.md) | Security |
| [adr/](./adr/README.md) | Architecture Decision Records |

When you change behavior, update the matching doc in the same PR. Significant decisions: add an ADR in `docs/adr/`.

## Repository layout (target)

Early work may use a **monorepo**; the north star is four repos (`nomabot-desktop`, `nomabot-firmware`, `nomabot-sdk`, `nomabot-assets`). See [Platform & Ecosystem](./13_PLATFORM.md).

## License

By contributing, you agree your contributions are licensed under the project license (to be specified in repository `LICENSE` file).

---

Questions? Open a discussion or issue on the project repository.
