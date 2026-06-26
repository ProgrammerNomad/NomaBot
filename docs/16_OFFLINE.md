# Offline Mode

> **Status:** Design specification - core product requirement, not a degraded fallback.

## Overview

NomaBot must work **fully** with only USB between desktop and device. No Wi-Fi, no internet, no AI, no GitHub, no Spotify, no cloud-still a complete, satisfying experience.

**Offline mode is a selling point**, not an error state.

## What works offline (USB only)

| Feature | Offline | Notes |
|---------|---------|-------|
| Display + animations | ✓ | Compiled packs on device |
| Activity detection | ✓ | Local OS APIs |
| Scheduler reminders | ✓ | SQLite jobs, local templates |
| Character + theme switch | ✓ | Pre-installed compiled packs |
| Settings + tray | ✓ | |
| Log viewer | ✓ | Local files only |
| Git plugin | ✓ | Local repo read |
| Spotify / cloud plugins | ✗ | Gracefully disabled |
| AI smart messages | ✗ | Template fallback |
| OTA / chunk upload | ✗ | USB serial sync only |
| Noma Store browse | ✗ | Local install folder only |

## Offline capability matrix

```text
                    Online optional    Offline required
                    ─────────────────  ────────────────
Render pipeline     ·                  ✓
Noma Runtime        ·                  ✓
Scheduler           ·                  ✓
Activity → idle     ·                  ✓
Template messages   ·                  ✓
Plugin (local)      ·                  ✓
Plugin (network)    ✓                  ·
AI providers        ✓                  ·
WebSocket/MQTT      ✓                  ·
Asset streaming     ✓                  USB compile + sync
```

## User experience

When offline:

1. Tray shows **Offline OK** (neutral)-not warning yellow unless device unplugged
2. Network plugins show "Needs internet" in Settings-disabled, not crashing
3. Scheduler still fires morning/evening with **template strings** from character personality
4. AI settings hidden or greyed with "Available when online"

See [UX](./15_UX.md) for emotional tone.

## Technical behavior

### Transport fallback

```text
Priority: configured transport → Serial fallback if device USB connected
```

`DeviceManager` marks device `online_via: serial` even when Wi-Fi config exists but LAN unreachable.

### Plugin declaration

Plugins declare in manifest:

```json
{
  "requires_network": true
}
```

Core skips enable without network-or user enables knowing it stays idle.

### AI fallback chain

```text
AI enabled + online  → AIService
AI enabled + offline → personality templates from pack
AI disabled          → templates only
```

Templates example (`personality.templates.good_morning`):

```yaml
- "Good morning."
- "Ready when you are."
```

### Asset updates offline

```text
User drops pack folder → CharacterService validates → build-assets locally → USB sync_begin/chunk
```

No CDN required.

## Configuration

`config.json`:

```json
{
  "offline": {
    "prefer_usb_when_connected": true,
    "disable_network_plugins_when_offline": true,
    "show_offline_badge": false
  }
}
```

## Testing offline

CI job `offline-smoke`:

- Block network (`pytest` marker `@offline`)
- Mock `ActivityService` + `SchedulerService`
- Assert render queue receives template message
- Assert Spotify plugin does not call HTTP

See [Testing Strategy](./14_TESTING.md).

## Related documentation

- [UX](./15_UX.md)
- [Architecture - Noma Runtime](./01_ARCHITECTURE.md#noma-runtime)
- [Communication - Serial transport](./04_COMMUNICATION.md)
- [Platform - Telemetry off by default](./13_PLATFORM.md#telemetry)
