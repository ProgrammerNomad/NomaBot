# Eyes Ambient

Standalone **ESP32 desk pet** for the [LILYGO T-Display S3](https://wiki.lilygo.cc/products/t-display-series/t-display-s3/). Cyan square eyes blink and look around on their own. WiFi provides live clock and weather - no PC or desktop app required.

Inspired by [Desper](https://www.youtube.com/watch?v=DZxC5BGkKT8).

## What you see

Three screens auto-rotate (Eyes → Clock → Weather → Eyes):

| Mode | Duration | Display |
|------|----------|---------|
| **Eyes** | ~45 s | Large cyan eyes on black background |
| **Clock** | ~8 s | Time + date |
| **Weather** | ~8 s | Temperature, condition, city |

Press **GPIO14** (user button) to cycle screens immediately.

> **Hold landscape:** Long edge horizontal, USB to the side. Portrait hold makes eyes stack vertically and look wrong.

## Hardware

| Item | Detail |
|------|--------|
| Board | LILYGO T-Display S3 (ESP32-S3, 320×170 landscape) |
| Power | USB-C |
| Reset | RST button |
| Flash | BOOT (GPIO0) + USB |
| Cycle screens | GPIO14 button |

## First-time setup

### 1. Install tools

- [Python 3.13+](https://www.python.org/) with [uv](https://docs.astral.sh/uv/)
- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- USB driver for ESP32-S3

### 2. Clone and sync

```powershell
git clone <your-repo-url>
cd NomaBot
uv sync
```

### 3. WiFi and weather credentials

```powershell
copy firmware\secrets.example.h firmware\secrets.h
```

Edit `firmware\secrets.h`:

```cpp
#define WIFI_SSID "YourNetwork"
#define WIFI_PASS "YourPassword"
#define WEATHER_API_KEY "your_openweathermap_key"   // free at openweathermap.org
#define WEATHER_CITY "Ghaziabad,IN"                 // City,CC - no space after comma
#define TIMEZONE_OFFSET_SEC 19800                   // IST = 19800
```

`secrets.h` is gitignored. You must re-flash firmware after any change.

### 4. Build art, compile pack, flash

```powershell
just assets
just flash
```

Or step by step:

```powershell
uv run python scripts/generate_eyes_art.py
uv run python -c "from pathlib import Path; from nomabot.assets.compiler import compile_pack; compile_pack(Path('assets/characters/eyes'), Path('compiled/eyes'), 'lilygo_tdisplay_s3_landscape')"
python scripts/copy_pack_to_firmware_data.py
cd firmware
pio run -e lilygo_tdisplay_s3 -t upload -t uploadfs --upload-port COM3
```

Replace `COM3` with your port.

### 5. Run

Power from USB. Press **RST** once after flashing. Eyes should animate within a second. Weather appears after WiFi connects (~30 s on first boot).

## Troubleshooting

| Problem | Fix |
|---------|-----|
| Eyes stacked vertically / tiny | Hold board **landscape** (long edge horizontal) |
| Eyes frozen | Reflash latest firmware (`just flash`) |
| Weather blank | Check `WEATHER_CITY` format (`City,CC`, no space), API key, serial log |
| Weather HTTP 401 | Invalid API key - wait ~10 min after creating a new OpenWeatherMap key |
| Port busy on flash | Close serial monitor; kill other `pio`/Python processes |
| Red boot screen | LittleFS pack missing - run `just assets` then `uploadfs` |

### Serial monitor (debug)

```powershell
cd firmware
pio device monitor -b 115200
```

Look for: `WiFi OK`, `Weather: 28C Rain Ghaziabad,IN`, `render_mode=eyes`.

## Project layout

```
assets/characters/eyes/   Source art + behavior.yaml
firmware/                 PlatformIO ESP32 firmware
  secrets.h               Your WiFi/API keys (not in git)
  data/                   LittleFS pack (generated)
sdk/                      Python asset compiler + tests
scripts/                  Art generation + pack copy
profiles/                 Device profile (320×170 landscape)
```

## Development

```powershell
just test          # run SDK tests
just firmware      # build firmware only
just assets        # regenerate eyes pack
just ci            # lint + test + build
```

Regenerate eyes art after editing `scripts/generate_eyes_art.py` or sprites, then `just assets` and re-flash.

## License

License TBD.
