# SDK

> **Status:** Design specification - publish as `nomabot-sdk` package / repository.

## Overview

NomaBot is a **platform**; contributors need clear APIs-not ad hoc copies of internal modules. The SDK repository (`nomabot-sdk`) is the single source of truth for protocol schemas, CLI tools, and author libraries.

```text
nomabot-sdk/
├── python/           # Python SDK - protocol client, types, test harness
├── plugin/           # Plugin SDK - base class, manifest schema
├── character/        # Character SDK - validator, compiler lib, editor backend
├── firmware/         # Firmware SDK - parser tests, renderer mocks
├── schemas/          # JSON Schema for protocol, packs, plugins, themes
├── cli/              # nomabot command entry points
└── docs/             # API reference generated from schemas
```

Desktop and firmware repos depend on released SDK versions (semver), not floating monorepo paths.

## High-level Developer API

Most authors should not hand-build JSON. The SDK exposes **`NomaBot`**-a friendly facade over protocol + runtime:

```python
from nomabot import NomaBot

bot = NomaBot.connect_serial("COM7")   # or connect(device_id="office")

await bot.play_animation("dance")
await bot.say("Hello")                 # show_message + personality tone
await bot.set_accessory("laptop")
await bot.notify("Build passed", title="CI")

# Multi-device
await bot.office.say("Good morning.")
await bot.broadcast.play_animation("sleep")
```

| Method | Maps to |
|--------|---------|
| `play_animation(id, **kw)` | `play_animation` + optional graph event |
| `say(text, style="speech")` | `show_message` |
| `set_state(name)` | `set_state` graph event |
| `notify(body, title=…)` | `show_notification` |

Desktop plugins use **`PluginContext.runtime`** (same types). Scripts use **`NomaBot`** directly. Both sit above `nomabot.protocol`.

### Low-level API (still available)

```python
from nomabot.client import NomaClient
from nomabot.protocol import Command

async with NomaClient.serial("COM3") as dev:
    await dev.send(Command.play_animation("coding", loop=True))
```

Use low-level API for protocol debugging; prefer **`NomaBot`** for integrations.

## Python SDK

**Audience:** Desktop contributors, third-party automation, Home Assistant bridges.

### Packages

```bash
pip install nomabot-sdk
```

### Capabilities

| Module | Purpose |
|--------|---------|
| `nomabot.protocol` | Envelope builders, version negotiation, parse/validate |
| `nomabot.client` | Transport-agnostic device client |
| `nomabot.device` | DeviceManager types, multi-device routing |
| `nomabot.events` | Event dataclass definitions (mirror desktop bus) |
| `nomabot.testing` | Mock transport, golden JSON fixtures |

### Example

```python
from nomabot.client import NomaClient
from nomabot.protocol import Command

async with NomaClient.serial("COM3") as dev:
    await dev.send(Command.play_animation("coding", loop=True))
    status = await dev.get_status()
```

Use Python SDK in integration tests so protocol drift is caught in CI before firmware ships.

## Character SDK

**Audience:** Character artists, pack publishers, CI validators.

### CLI tools

```bash
nomabot character init my_fox --template fox
nomabot character validate ./characters/my_fox
nomabot build-assets --input ./characters/my_fox --output ./compiled/my_fox
nomabot character pack ./compiled/my_fox -o my_fox.nomachar
```

### Libraries

| API | Purpose |
|-----|---------|
| `CharacterValidator` | Schema + graph + ref integrity |
| `AnimationGraph` | Load/edit graph programmatically |
| `AssetCompiler` | PNG → binary pipeline stages |
| `PreviewRenderer` | Desktop software renderer (same layer rules as firmware) |

Character Editor (PySide6) imports `PreviewRenderer` and `AnimationGraph`-no duplicated logic.

### Pack template

```text
templates/fox/
├── config.json
├── animation_graph.json
├── animations/idle.json
└── sprites/.gitkeep
```

## Plugin SDK

**Audience:** Integration authors (Git, Spotify, Discord, …).

### Manifest schema

Validated at load time from `schemas/plugin.manifest.json`.

```json
{
  "id": "com.example.spotify",
  "name": "Spotify",
  "version": "1.0.0",
  "entry": "plugin:SpotifyPlugin",
  "min_desktop": "0.4.0",
  "min_sdk": "0.2.0",
  "permissions": ["internet", "notifications"],
  "subscriptions": ["activity.changed", "scheduler.fire"]
}
```

### Base class

```python
from nomabot.plugin import Plugin, PluginContext, RenderSpec

class SpotifyPlugin(Plugin):
    def on_enable(self) -> None:
        self.subscribe("activity.changed", self.on_activity)

    def on_activity(self, event) -> None:
        if self.now_playing:
            self.context.request_render(RenderSpec(
                animation="music",
                accessory="headphones",
            ))
```

### Permissions (declared capabilities)

| Permission | Grants |
|------------|--------|
| `filesystem.read` | Read paths via scoped APIs |
| `filesystem.write` | Plugin data dir only |
| `internet` | Outbound HTTP/WebSocket |
| `git` | GitService metadata |
| `notifications` | OS notifications + device banners |
| `ai.query` | AIService completions |
| `scheduler.register` | Register cron jobs |

Users approve permissions in UI before enable. Plugin SDK documents each flag.

## Firmware SDK

**Audience:** Firmware porters, display driver contributors.

### Contents

| Artifact | Purpose |
|----------|---------|
| `protocol_parser` | Host-side C++ or Python golden tests |
| `renderer_mock` | Headless framebuffer for animation tests |
| `schema v1` | Command enum code generation (optional) |
| `test_vectors/` | NDJSON fixtures every firmware build must pass |

Firmware repo runs:

```bash
nomabot firmware test-vectors --port /dev/ttyUSB0
```

against hardware or emulator.

### Renderer interface (C++ concept)

```cpp
class Renderer {
 public:
  virtual void beginFrame() = 0;
  virtual void blitSprite(const Sprite& s, int x, int y) = 0;
  virtual void endFrame() = 0;
};
// St7789Renderer, OledRenderer, …
```

See [Firmware](./03_FIRMWARE.md).

## CLI umbrella

```bash
nomabot --help

Commands:
  build-assets      Compile character pack for device profile
  build-theme       Compile theme bundle
  character init    Scaffold new pack
  character validate Validate source pack
  character pack    Build .nomachar / .nomabundle release artifact
  protocol lint     Validate NDJSON fixture files
  firmware test-vectors  Run protocol conformance tests
```

Entry point: `nomabot-sdk` PyPI package, versioned independently from desktop.

## Versioning and compatibility

| Artifact | Version field | Compatibility |
|----------|---------------|---------------|
| Protocol | `"v": 1` in JSON | Negotiated on hello |
| SDK | semver PyPI | `min_sdk` in plugin manifest |
| Character pack | `metadata.version` | `min_firmware`, profile |
| Compiled bundle | manifest hash | Exact match for activate |

Breaking SDK release → major bump + migration guide in `nomabot-sdk/docs/CHANGELOG.md`.

## Distribution

| Phase | Model |
|-------|--------|
| Bootstrap | `sdk/` folder in monorepo |
| Platform | `nomabot-sdk` GitHub repo + PyPI |
| Assets | `nomabot-assets` releases attach `.nomabundle` binaries |

Third-party tools should **only** depend on published SDK-not `desktop/core/` internals.

## Related documentation

- [Architecture](./01_ARCHITECTURE.md)
- [Asset Pipeline](./11_ASSET_PIPELINE.md)
- [Plugin System](./07_PLUGIN_SYSTEM.md)
- [Character System](./06_CHARACTER_SYSTEM.md)
- [Communication](./04_COMMUNICATION.md)
