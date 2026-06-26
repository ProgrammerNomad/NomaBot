# Platform & Ecosystem

> **Status:** Design specification — ecosystem features roll out after core platform stabilizes.

## Overview

NomaBot is designed as an **extensible platform** from day one: multiple devices, themes, compiled assets, SDKs, and a future marketplace. This document covers cross-cutting product concerns not tied to a single layer.

## Multi-repository strategy

**Target topology** (north star even if bootstrap starts as monorepo):

| Repository | Scope | Release cadence |
|------------|-------|-----------------|
| `nomabot-desktop` | PySide6 app, services, UI, transport | Frequent |
| `nomabot-firmware` | ESP32, renderer backends, OTA | Conservative |
| `nomabot-sdk` | Schemas, CLI, all SDKs | When contracts change |
| `nomabot-assets` | Characters, themes, sounds | Content-driven |

### Why split?

- **Independent release cycles** — ship Spotify plugin desktop update without firmware bump
- **Focused issue trackers** — art contributors avoid firmware labels
- **Smaller clones** — plugin dev: `git clone nomabot-sdk` + pip install desktop dep
- **Clear boundaries** — third parties depend on SDK semver, not git subpaths

### Monorepo bootstrap

Until milestone 2 exits, a single repo may mirror:

```text
NomaBot/
├── desktop/    → future nomabot-desktop
├── firmware/   → future nomabot-firmware
├── sdk/        → future nomabot-sdk
├── assets/     → future nomabot-assets
└── docs/
```

CI should tag subpaths and validate cross-repo contracts as if already split.

## Multi-device product experience

Users own multiple ESP32 devices. **Device Manager** (desktop UI + `DeviceManager` service) is a first-class feature—not a later addon.

| Scenario | Configuration |
|----------|---------------|
| Office desk | `nomabot` + git plugin reactions |
| Living room | `coding_cat` + weather theme |
| Workbench | `astronaut` + minimal theme, serial only |

Each device: unique id, friendly name, character, theme override (optional), transport config.

Broadcast commands (e.g. “Good night” scheduler job) can target **all devices** or a **room group** (future).

## Theme system

Characters define *who*; themes define *how it looks*.

```text
themes/
├── cyberpunk/
├── minimal/
├── retro/
├── dark/
└── nature/
```

### Theme package layout

```text
themes/cyberpunk/
├── theme.json
├── fonts/
├── icons/
├── sounds/
└── preview.png
```

### `theme.json` (conceptual)

```json
{
  "id": "cyberpunk",
  "name": "Cyberpunk",
  "version": "1.0.0",
  "palette": {
    "bubble_bg": "#1a0a2e",
    "bubble_text": "#00ffcc",
    "notification_accent": "#ff006e"
  },
  "font": "fonts/px.ttf",
  "bubble_style": "rounded_neon",
  "background_filter": "scanlines",
  "sounds": {
    "notify": "sounds/chirp.wav"
  }
}
```

Themes affect:

- Speech bubble shapes and colors
- Notification banner styling
- Default background filters (not character body art)
- UI accent in desktop app when “match device theme” enabled

Compile with `nomabot build-theme` ([Asset Pipeline](./11_ASSET_PIPELINE.md)).

### Theme Editor (planned)

Mirror of Character Editor for themes—**Theme Editor** (PySide6):

- Edit palette tokens, font, bubble style visually
- Preview notification + speech bubble on 170×320 canvas
- Export `theme.json` + assets → `nomabot build-theme`

Ship target: Milestone 4 alongside first official themes. Shares SDK preview renderer with Character Editor.

## Telemetry

Telemetry is **anonymous, off by default, opt-in only**.

### Principles

| Rule | Detail |
|------|--------|
| Default | **Off** — zero phone-home on first install |
| Opt-in | Settings → Privacy → "Send anonymous usage statistics" |
| Transparency | In-app list of exact fields before enable |
| No content | Never activity titles, git data, messages, or AI prompts |
| Local logs | Always available regardless of telemetry ([Logging](./02_DESKTOP_APP.md#logging)) |

### If enabled (example payload)

```json
{
  "event": "app_start",
  "desktop_version": "0.2.0",
  "os": "Windows 11",
  "device_count": 1,
  "plugin_count_enabled": 2,
  "locale": "en-US"
}
```

Optional crash reports (separate toggle): stack trace hash, version, OS—no user files.

### Implementation notes

- Batch upload at most once per 24 h when online
- Disabled in offline mode ([Offline Mode](./16_OFFLINE.md))
- Document in [Security](./17_SECURITY.md)

## Noma Store (marketplace)

Design catalog semantics **now**, implement as GitHub Releases + JSON index initially.

```text
Noma Store (evolution)
├── Characters      .nomachar / .nomabundle
├── Plugins         pip package or .nomaplugin zip
├── Themes          .nomatheme
└── Animation packs  graph-only DLC for existing characters
```

### Catalog entry (conceptual)

```json
{
  "id": "com.nomabot.character.coding_cat",
  "type": "character",
  "version": "2.1.0",
  "download_url": "https://…",
  "hash": "sha256:…",
  "min_desktop": "0.3.0",
  "min_firmware": "0.2.0",
  "protocol_max": 1,
  "license": "MIT",
  "author": "…"
}
```

### Trust model (phased)

| Phase | Trust |
|-------|-------|
| v1 | Official repo + community PR review |
| v1.5 | Signed artifacts (maintainer keys) |
| v2 | Contributor signing + store moderation |

Desktop “Browse catalog” UI reads index URL; installs via same validator/compiler pipeline as local folders.

## Extensibility summary

| Extension | Install feels like | Built with |
|-----------|-------------------|------------|
| Character | VS Code theme | Character SDK + Editor |
| Plugin | Browser extension | Plugin SDK |
| Theme | UI skin | Theme compiler |
| Transport | Driver plugin | Python SDK transport registry (advanced) |
| Renderer | Display driver | Firmware SDK |

## Commercial potential

Platform choices that support commercialization without fork:

- Signed firmware + signed asset bundles
- Premium character/theme catalog
- Enterprise plugin tier (Jira, SSO audit)
- Hardware reference design + licensed enclosure
- SDK remains open; store curates discovery

OSS core + paid catalog is compatible with MIT core license (TBD).

## Related documentation

- [Architecture](./01_ARCHITECTURE.md)
- [Asset Pipeline](./11_ASSET_PIPELINE.md)
- [SDK](./12_SDK.md)
- [Roadmap](./10_ROADMAP.md)
- [Vision](./00_VISION.md)
