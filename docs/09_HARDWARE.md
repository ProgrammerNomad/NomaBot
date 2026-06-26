# Hardware

> **Status:** Design specification — **official prototype: LILYGO T-Display S3**.

## Overview

NomaBot firmware and asset profiles target **real hardware**, not a generic placeholder. The **official supported device** for development and documentation is the **LILYGO T-Display S3** (ESP32-S3, 170×320 ST7789). Other boards are experimental community ports.

## Supported devices

### Official ✓

| Device | MCU | Display | Profile id | Status |
|--------|-----|---------|------------|--------|
| **LILYGO T-Display S3** | ESP32-S3 | **170×320** ST7789 IPS | `lilygo_tdisplay_s3` | **Primary prototype** |

Characteristics:

- USB-C (power + USB CDC serial)
- Onboard ST7789, portrait 170×320
- LovyanGFX board profile: `LilyGo T-Display-S3`
- Default asset compile profile: **170×320** (not 240×240)

### Experimental (community)

| Device | Display | Profile id | Notes |
|--------|---------|------------|-------|
| ESP32-2432S028 | 320×240 | `esp32_2432s028` | Cheap round-corner panel |
| ESP32-2432S024 | 240×320 | `esp32_2432s024` | Variant pinout |
| Waveshare ESP32-S3 LCD | varies | `waveshare_*` | Per module docs |
| Generic ST7789 240×240 | 240×240 | `st7789_240_square` | Reference only |

Experimental devices are not CI-gated until promoted to official.

## LILYGO T-Display S3 — software reference

### Specifications

| Parameter | Value |
|-----------|-------|
| Resolution | **170 × 320** (portrait) |
| Driver | ST7789 |
| Interface | SPI via LovyanGFX |
| USB | Native USB CDC (ESP32-S3) |
| Flash | 16 MB (typical board) |
| PSRAM | 8 MB (typical) |

### Pin mapping

Use LovyanGFX **`LilyGo T-Display-S3`** preset as source of truth—do not duplicate stale GPIO tables here. Firmware config:

```text
firmware/src/renderer/boards/lilygo_tdisplay_s3.hpp
```

Rebuild when LilyGo or LovyanGFX updates preset.

### Asset implications

Character packs default to profile `lilygo_tdisplay_s3`:

- Safe area: center 170×170 or full 170×320 depending on layout
- Compile: `nomabot build-assets --profile lilygo_tdisplay_s3`
- Preview in Character Editor at **170×320**

Packs authored for `st7789_240_square` require re-export for official hardware.

## Block diagram

```text
   USB-C ──► ESP32-S3 (LILYGO T-Display S3)
                 │
                 ├── SPI ──► ST7789 170×320
                 ├── USB CDC ──► Serial transport (NDJSON)
                 └── Wi-Fi ──► WebSocket / MQTT (milestone 5)
```

## Design goals

| Goal | Implication |
|------|-------------|
| Desk-friendly | Compact portrait display beside monitor |
| Developer-accessible | Popular board, abundant docs |
| USB-first | Works fully offline ([Offline Mode](./16_OFFLINE.md)) |
| Expandable | Experimental port via renderer abstraction |

## Power

| Parameter | Spec |
|-----------|-------|
| Input | 5 V USB-C |
| Typical draw | 100–300 mA (backlight dependent) |

## Connectivity

### USB Serial (MVP)

- ESP32-S3 USB CDC → COM port on Windows
- NDJSON protocol at 115200 baud default ([Communication](./04_COMMUNICATION.md))
- Primary path for development and offline use

### Wi-Fi (milestone 5)

- Station mode + pairing ([Security](./17_SECURITY.md))
- WebSocket / MQTT on LAN

## Optional peripherals

| Peripheral | LILYGO T-Display S3 |
|------------|---------------------|
| Onboard button | Boot/GPIO0 — map in firmware caps |
| Touch | Not on base S3 model |
| SD | Not onboard — use flash streaming |

## Hardware profiles (software)

Firmware, compiler, and packs declare profile:

```json
{
  "hardware_profiles": ["lilygo_tdisplay_s3"],
  "display": { "width": 170, "height": 320 }
}
```

Desktop warns when pack profile mismatches connected device.

## Flashing firmware

```text
1. PlatformIO or Arduino: board = LilyGo T-Display-S3
2. Select renderer backend lilygo_tdisplay_s3
3. Flash via USB-C
4. nomabot mock-device or desktop connects over CDC
5. Verify hello_ack reports 170×320
```

## Mechanical

Enclosure STLs (future): `hardware/enclosure/lilygo_tdisplay_s3/` — portrait stand, cable relief.

## Related documentation

- [Firmware — Renderer abstraction](./03_FIRMWARE.md)
- [ADR 0003 — Renderer abstraction](./adr/0003-renderer-abstraction.md)
- [Asset Pipeline — compile profiles](./11_ASSET_PIPELINE.md)
- [Security — pairing](./17_SECURITY.md)
