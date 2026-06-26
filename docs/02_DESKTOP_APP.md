# Desktop Application

> **Status:** Design specification - implementation not yet started.

## Purpose

The desktop application is NomaBot's **control plane**. It observes the user's environment, runs plugins, schedules timed messages, manages multiple devices, and sends render commands to ESP32 hardware. It is a long-running PySide6 app with system tray presence and optional settings windows.

## Technology - PySide6 from day one

| Piece | Choice | Notes |
|-------|--------|-------|
| Language | Python 3.13 | |
| UI framework | **PySide6 (Qt6)** | **Only** UI framework - no CustomTkinter, no migration path |
| UI design | Qt Designer | `.ui` files loaded at runtime |
| Packaging | Nuitka | Native Windows `.exe` |
| Config | JSON | User + defaults |
| Database | SQLite | Devices, settings, scheduler, plugin state |

There is no “v1 with a simpler UI” plan. Every surface-tray, settings, device manager, character editor, log viewer-is built on PySide6 from milestone 2 onward.

## Modular layout (not monolithic)

```text
desktop/
├── core/               # Noma Core - event bus, app bootstrap, interfaces
│   ├── bus.py
│   ├── lifecycle.py
│   └── interfaces/
├── services/           # One module per service; no cross-imports
│   ├── activity.py
│   ├── scheduler.py
│   ├── device_manager.py
│   ├── character.py
│   ├── config.py
│   └── ai.py
├── plugins/            # Bundled plugins (Git, Spotify, …)
├── ui/                 # PySide6 only
│   ├── forms/          # Qt Designer .ui
│   ├── widgets/
│   ├── tray/
│   └── tools/          # Character Editor, Log Viewer panels
├── transport/          # Transport interface + adapters
├── storage/            # SQLite, migrations
└── utils/              # Logging, paths, shared types
```

```text
                    Noma Core
                        │
        ┌───────────────┼───────────────┐
        ▼               ▼               ▼
   Services        Plugins            UI
        │               │               │
        └───────────────┴───────────────┘
                        │
                   Event Bus only
                   (services never import each other)
```

## Layered architecture

```text
┌─────────────────────────────────────────────────────────┐
│  UI - tray, settings, device manager, log viewer        │
└───────────────────────────┬─────────────────────────────┘
                            │ events only
┌───────────────────────────▼─────────────────────────────┐
│  Event Bus                                              │
└─▲───────────────▲───────────────▲───────────────▲───────┘
  │               │               │               │
  Services    Plugins      DeviceManager    SchedulerService
  │               │               │               │
  └───────────────┴───────────────┴───────────────┘
                            │
              Transport Layer · Storage · Logging
```

**Rules:**

- UI never imports plugin modules directly
- Services never import sibling services
- Plugins use Plugin SDK + event bus; submit via **Noma Runtime** only
- No direct transport or JSON from plugins

## Noma Runtime

Central orchestrator ([Architecture](./01_ARCHITECTURE.md#noma-runtime), [ADR 0004](./adr/0004-noma-runtime.md)):

```text
Plugins / Scheduler / Services
            │
            ▼
       Event Bus
            │
            ▼
      Noma Runtime
       ├── Scheduler queue
       ├── Render queue (priority)
       ├── Animation queue
       └── DeviceManager → Transport
```

```python
# Plugin - never call device directly
self.context.runtime.submit(RenderRequest(
    device_id="office",
    animation="coding",
    priority=Priority.NORMAL,
))
```

Priority levels: `CRITICAL`, `HIGH`, `NORMAL`, `LOW`, `BACKGROUND` - see [ADR 0005](./adr/0005-event-priority.md) and [UX](./15_UX.md).

## UI layer

### Responsibilities

- System tray and quick actions
- Settings (plugins, AI, privacy, themes)
- **Device Manager** - multi-device list, per-device character and transport
- **Character Manager** - install packs, preview
- **Character Editor** - authoring tool (see [Asset Pipeline](./11_ASSET_PIPELINE.md))
- **Log Viewer** - filter `desktop.log`, `firmware.log`, `plugins.log`, `transport.log`

### Key surfaces

| Surface | Description |
|---------|-------------|
| **System tray** | Connect/disconnect, device switcher, mute |
| **Settings** | Tabbed: General · Devices · Characters · Plugins · AI · Themes · Logs |
| **Device Manager** | Name devices, assign characters, transport per device |
| **Character Editor** | Timeline, graph, layer preview, export |
| **Log Viewer** | Live tail, level filter, export for bug reports |

### Qt Designer workflow

1. Design in Qt Designer → `desktop/ui/forms/*.ui`
2. Load via `QUiLoader` or `pyside6-uic`
3. Widgets in `desktop/ui/widgets/`; view-models subscribe to bus
4. One `.ui` file per dialog/panel - no mega-main-window

## Event bus

Async-friendly pub/sub with typed event dataclasses.

### Example topics

| Topic | Payload (conceptual) | Publishers | Subscribers |
|-------|----------------------|------------|-------------|
| `activity.changed` | app id, title, idle | ActivityService | Plugins, UI |
| `render.request` | device_id, layers, message | Plugins, Scheduler | DeviceManager |
| `device.connected` | device_id, transport | Transport | UI, Services |
| `device.disconnected` | device_id, reason | Transport | UI |
| `scheduler.fire` | job_id, payload | SchedulerService | Plugins, render |
| `scheduler.register` | job spec | Plugins, Services | SchedulerService |
| `theme.changed` | theme_id | ConfigService | UI, render |
| `ai.response` | text, metadata | AIService | Plugins |

### Design rules

- Events are **immutable**
- Handlers must not block the UI thread
- Unknown topics logged in debug builds
- Plugins subscribe only to manifest-declared topics

## Service layer

Each service is a single module with a narrow API. **Services do not import each other.**

| Service | Responsibility |
|---------|----------------|
| `ConfigService` | JSON config load/save, migrations |
| `StorageService` | SQLite access patterns |
| `SchedulerService` | Central cron-like job registry (see below) |
| `DeviceManager` | Multi-device registry, routing, command queues |
| `ActivityService` | Foreground window, idle detection |
| `CharacterService` | Pack install, validate, compile hook |
| `ThemeService` | Active theme, token resolution |
| `AIService` | Provider abstraction ([AI](./08_AI.md)) |
| `LoggingService` | File handlers, firmware log mirror |

Integration logic (Git, Spotify, …) lives in **plugins**, not core services, unless promoted deliberately with ADR.

### SchedulerService

**One scheduler for the entire app.** No `threading.Timer` in plugins.

```python
# Conceptual - plugins register via event or API
scheduler.register(
    job_id="good-morning",
    cron="0 8 * * *",
    event="scheduler.fire",
    payload={"template": "good_morning"},
)
```

Daily examples:

| Time | Event payload |
|------|----------------|
| 08:00 | Good morning message |
| 14:00 | Hydration reminder |
| 18:00 | Walk reminder |
| 22:30 | Good night |

Scheduler persists jobs in SQLite; fires `scheduler.fire` on the bus. Display logic stays in plugins or render coalescer.

### DeviceManager

Manages **N devices** with independent state:

```json
{
  "device_id": "esp32-office-a1",
  "name": "Office",
  "character_id": "nomabot",
  "transport": { "type": "websocket", "host": "192.168.1.50" },
  "online": true
}
```

| Operation | Behavior |
|-----------|----------|
| `send(device_id, command)` | Route to device's transport queue |
| `broadcast(command)` | All online devices |
| `assign_character(device_id, pack)` | Sync compiled assets, `load_character` |

UI **Device Manager** panel edits this registry.

## Plugin layer

See [Plugin System](./07_PLUGIN_SYSTEM.md). Bundled in `desktop/plugins/`; user plugins in `%APPDATA%/NomaBot/plugins/`.

## Transport layer

Not hardcoded USB/Wi-Fi/MQTT - implements shared **Transport interface**:

```python
class Transport(Protocol):
    async def connect(self, config: TransportConfig) -> None: ...
    async def disconnect(self) -> None: ...
    async def send(self, data: bytes) -> None: ...
    def on_receive(self, callback: Callable[[bytes], None]) -> None: ...
```

Implementations: `SerialTransport`, `WebSocketTransport`, `MqttTransport`, `BleTransport` (future), `TcpTransport` (future).

`DeviceManager` binds one transport instance per device. Details: [Communication](./04_COMMUNICATION.md).

## Logging

Structured logs from first desktop release:

```text
%APPDATA%/NomaBot/logs/
├── desktop.log       # core, services, UI
├── firmware.log      # mirrored device logs
├── plugins.log       # per-plugin child loggers
└── transport.log     # connect, send, parse errors
```

| Logger | Example content |
|--------|-----------------|
| `noma.desktop` | Startup, config migrations |
| `noma.transport.serial` | COM open, NDJSON parse errors |
| `noma.plugin.git` | Repo detection failures |
| `noma.firmware` | Mirrored ESP32 `log` events |

**Log Viewer** (Settings → Logs): tail files, level filter, clear, export bundle for issues.

Rotation: 5 MB × 5 files per channel (configurable).

## Storage layer

### SQLite tables (conceptual)

| Table | Purpose |
|-------|---------|
| `settings` | Key-value |
| `devices` | DeviceManager registry |
| `scheduler_jobs` | Cron jobs |
| `plugin_state` | Per-plugin data |
| `activity_log` | Optional AI context |

### File layout

```text
%APPDATA%/NomaBot/
├── config.json
├── noma.db
├── plugins/
├── characters/         # Installed source packs
├── compiled/           # Compiler output cache
├── themes/
└── logs/
```

## Application lifecycle

1. Init logging → config → SQLite → event bus
2. Start SchedulerService, DeviceManager, core services
3. Load plugins (permissions check)
4. System tray visible
5. Auto-connect registered devices (backoff per transport)
6. Quit: flush queues, disconnect, close logs

## Settings model

| Category | Examples |
|----------|----------|
| General | Start with Windows, language |
| Devices | Per-device name, transport, character |
| Characters | Default pack, editor path |
| Themes | Active theme ([Platform](./13_PLATFORM.md)) |
| Plugins | Enable, permissions review |
| AI | Provider, keys, privacy tier |
| Logs | Level, retention, open log folder |

## Error handling

| Failure | Behavior |
|---------|----------|
| Device offline | Queue or drop per device policy; tray badge |
| Plugin crash | Disable session; log to `plugins.log` |
| Scheduler misfire | Log + retry once |
| Compiler failure | Block pack activate; show validator output |

## Testing strategy

| Layer | Approach |
|-------|----------|
| Services | Mock bus; no cross-service imports to mock |
| Scheduler | Fake clock; fire events |
| DeviceManager | Multiple mock transports |
| Transport | Loopback serial / WS test server |
| UI | Smoke + critical path manual QA |

## Related documentation

- [Architecture](./01_ARCHITECTURE.md)
- [Noma Runtime - ADR 0004](./adr/0004-noma-runtime.md)
- [Communication](./04_COMMUNICATION.md)
- [Asset Pipeline](./11_ASSET_PIPELINE.md)
- [Testing Strategy](./14_TESTING.md)
- [Offline Mode](./16_OFFLINE.md)
- [Plugin System](./07_PLUGIN_SYSTEM.md)
