# Eyes Ambient

Standalone **ESP32 desk pet** for the [LILYGO T-Display S3](https://wiki.lilygo.cc/products/t-display-series/t-display-s3/). Cyan square eyes blink and look around on their own. WiFi provides live clock and weather - no PC or desktop app required.

Inspired by [Desper](https://www.youtube.com/watch?v=DZxC5BGkKT8).

**Version 0.6.0**

## What you see

Auto-rotating screens: **Eyes → Clock → Weather → Eyes**

| Mode | Default duration | Display |
|------|------------------|---------|
| **Eyes** | ~45 s | Animated cyan eyes (blink, look, rain when wet) |
| **Clock** | ~8 s | Time + date (POSIX timezone) |
| **Weather** | ~8 s | Temperature, condition, city (HTTPS OpenWeatherMap) |

### Button controls (GPIO14)

| Action | Result |
|--------|--------|
| **Short press** | Cycle Eyes / Clock / Weather |
| **Long press** (from Eyes) | Open **Pomodoro** timer (25 min) |
| **Short press** (in Pomodoro) | Start / pause timer |
| **Long press** (in Pomodoro) | Reset timer |
| **Long press** (from Clock/Weather) | **Stats** screen (uptime, heap, RSSI, FW version) |
| **Short press** (in Stats) | Return to Eyes |

Corner dots show service health: WiFi, clock, weather (green = OK, orange = stale, red = down).

> **Hold landscape:** Long edge horizontal, USB to the side.

## Hardware

| Item | Detail |
|------|--------|
| Board | LILYGO T-Display S3 (ESP32-S3, 320×170 landscape) |
| Power | USB-C |
| Reset | RST button |
| Flash | BOOT (GPIO0) + USB |
| Cycle screens | GPIO14 button |

## First-time setup

### Option A - Captive portal (recommended)

1. Flash firmware + LittleFS (`just flash`).
2. On first boot with no `wifi.json`, device opens WiFi AP **`EyesSetup`** (password `eyes1234`).
3. Connect phone/laptop, open any URL → setup form.
4. Enter WiFi, OpenWeatherMap key, city (`City,CC`), timezone (e.g. `IST-5:30`).
5. Device saves to LittleFS and reboots.

### Option B - Compile-time secrets (developers)

```powershell
copy firmware\secrets.example.h firmware\secrets.h
```

Edit `firmware\secrets.h`:

```cpp
#define WIFI_SSID "YourNetwork"
#define WIFI_PASS "YourPassword"
#define WEATHER_API_KEY "your_openweathermap_key"
#define WEATHER_CITY "Ghaziabad,IN"
#define TIMEZONE "IST-5:30"
```

`secrets.h` is gitignored. Re-flash after changes.

### Build and flash

```powershell
uv sync --all-packages
just flash
```

Replace `COM3` in PlatformIO upload port as needed.

### OTA updates (after WiFi connected)

```powershell
cd firmware
pio run -e lilygo_tdisplay_s3 -t upload --upload-port eyes-ambient.local
```

Device advertises mDNS hostname **`eyes-ambient.local`**.

## Troubleshooting

| Problem | Fix |
|---------|-----|
| Eyes stacked vertically | Hold board **landscape** |
| Eyes frozen | Reflash (`just flash`) |
| Weather blank | Check city format, API key, serial log |
| Weather HTTP 401 | Invalid or new API key (wait ~10 min after creation) |
| Red boot "WIFI SETUP" | Use captive portal or fill `secrets.h` |
| Port busy on flash | Close serial monitor |

### Serial monitor

```powershell
cd firmware
pio device monitor -b 115200
```

Look for: `WiFi OK`, `Weather: 28C Clear Ghaziabad,IN`, `mDNS: eyes-ambient.local`.

## Project layout

```
assets/characters/eyes/   Source art + behavior.yaml
firmware/                 PlatformIO ESP32 firmware
  secrets.h               Optional compile-time credentials
  data/                   LittleFS pack (generated)
sdk/                      Python asset compiler + tests
scripts/                  Art, version sync, secret checks
profiles/                 Device profile (320×170 landscape)
```

## Development

```powershell
just ci            # version sync + lint + test + firmware build
just test          # SDK tests only
just assets        # regenerate eyes pack
just firmware      # build firmware
```

Enable pre-commit secret check:

```powershell
git config core.hooksPath .githooks
```

## License

MIT - see [LICENSE](LICENSE).
