# NomaBot Vision

> **Status:** Design specification — project in early planning phase.

## What is NomaBot?

NomaBot is a **desk companion platform**: a small ESP32-powered display plus a PySide6 desktop control plane, an asset pipeline, SDKs, and an extensible plugin/character ecosystem.

The desktop application is the brain. The firmware is the display. Characters, themes, and integrations are **data and plugins**—not hardcoded features.

```
┌─────────────────┐         ┌─────────────────┐
│  Desktop App    │  JSON   │  ESP32 Device   │
│  (Python/Qt)    │ ──────► │  (Display only) │
│                 │  USB /  │                 │
│  Activity, AI,  │  Wi-Fi  │  Animations,    │
│  Plugins        │         │  Sprites, UI    │
└─────────────────┘         └─────────────────┘
```

## Why NomaBot exists

Modern work happens across dozens of tools—IDEs, chat apps, music, calendars, git, containers, home automation. Most desk gadgets show static information or require you to configure everything manually.

NomaBot aims to be different:

| Problem | NomaBot approach |
|--------|-------------------|
| Static desk toys | Live, layered animations tied to real activity |
| Vendor lock-in | Open character packs and plugin ecosystem |
| Cloud-only AI | Pluggable providers: cloud **or** fully local |
| Fragile firmware | Dumb device: firmware renders; desktop decides |
| One-size-fits-all character | Data-driven characters, swappable like themes |

## Design principles

These principles apply to every layer of the stack. When in doubt, prefer the option that best satisfies them.

### 1. Separation of concerns

The ESP32 **never** knows about Git, Spotify, VS Code, or weather APIs. It only knows how to:

- Play an animation
- Show a message
- Change accessory, background, or effects
- Display a notification

All domain logic lives in the desktop app.

### 2. Data over code

Characters, animations, accessories, and backgrounds are **packaged assets**, not firmware patches. Installing a new character should feel like installing a VS Code theme.

### 3. Plugins over monolith

Everything beyond core connectivity, rendering, and settings is a **plugin**. The core stays small; the ecosystem grows without bloating releases.

### 4. Transport-agnostic protocol

USB Serial, WebSocket, and MQTT share one **JSON command schema**. Changing transport must not require firmware changes.

### 5. Provider-agnostic AI

AI is an optional layer behind a single abstraction. Users choose OpenAI, Gemini, Claude, Ollama, LM Studio, or future providers without rewriting integrations.

### 6. Platform from day one

Invest early in the **asset compiler**, **SDKs**, **protocol versioning**, **animation graphs**, and **modular desktop core** (`Noma Core`). These foundations add modest upfront cost but prevent rewrites when adding characters, transports, or marketplace content.

### 7. PySide6 from day one

The desktop UI uses **PySide6 + Qt Designer** from the first release—no interim UI toolkit and no migration later.

### 8. Windows-first, cross-platform later

Primary target is Windows (where most developers work). PySide6 and Python keep a credible path to macOS and Linux without rewriting the architecture.

## Target audience

| Audience | What they get |
|----------|----------------|
| **End users** | A desk companion that reacts to their workflow out of the box |
| **Character authors** | A documented pack format to ship new mascots and animation sets |
| **Plugin developers** | SDK hooks to integrate new apps and services |
| **Hardware makers** | Renderer abstraction to port new displays |
| **Contributors** | Four-repo layout, SDKs, and contribution guidelines |

## What NomaBot is not

- **Not a smart display framework for arbitrary UI.** It is optimized for sprite-based characters and short messages.
- **Not a cloud service.** All core functionality runs locally; cloud AI is optional.
- **Not a single-character toy.** Multi-character support is a first-class design goal.
- **Not firmware-heavy.** OTA and rendering yes; business logic no.

## Success criteria

We will know the vision is realized when:

1. A user can plug in a device, install the desktop app, and see meaningful activity within minutes.
2. A third party can ship a character pack without forking firmware or the desktop core.
3. A third party can ship a plugin (e.g. Jira, Discord) using documented SDK interfaces.
4. Switching from USB to Wi-Fi requires no firmware rebuild—only configuration.
5. A user with **three devices** can assign different characters and transports per room.
6. `nomabot build-assets` produces device-ready packs artists validate in the **Character Editor**.
7. The project documentation is sufficient for a new contributor to set up, build, and submit a PR without oral tradition.

## Related documentation

| Document | Contents |
|----------|----------|
| [Architecture](./01_ARCHITECTURE.md) | System design, platform layout, multi-repo |
| [Asset Pipeline](./11_ASSET_PIPELINE.md) | Compiler, Character Editor, streaming |
| [SDK](./12_SDK.md) | Python, Character, Plugin, Firmware SDKs |
| [Platform & Ecosystem](./13_PLATFORM.md) | Themes, telemetry, store |
| [Testing Strategy](./14_TESTING.md) | CI, mocks, golden tests |
| [UX](./15_UX.md) | User experience and daily arc |
| [Offline Mode](./16_OFFLINE.md) | USB-only complete product |
| [Security](./17_SECURITY.md) | Pairing, OTA, sandbox |
| [ADR index](./adr/README.md) | Architecture decisions |
| [Roadmap](./10_ROADMAP.md) | Milestones and implementation order |
| [Contributing](./CONTRIBUTING.md) | How to participate |

---

*Last updated: project planning phase.*
