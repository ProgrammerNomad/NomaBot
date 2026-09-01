# Eyes Ambient — Standalone Desk Pet

Landscape **eyes-only** pet inspired by [Desper](https://www.youtube.com/watch?v=DZxC5BGkKT8). Branch: `feature/eyes-ambient`.

**No desktop app required.** The ESP32 connects to WiFi, fetches time (NTP) and weather (OpenWeatherMap), and animates eyes on its own.

## What you see

Three screens **auto-rotate** (Eyes → Clock → Weather → Eyes):

| Mode | LCD |
|------|-----|
| **Eyes** (boot default, ~45s) | Black background, cyan square eyes blink/look |
| **Clock** (~8s) | Large centered time + date |
| **Weather** (~8s) | Large temp, condition, city |

**Press the user button (GPIO14)** to cycle modes immediately. Hold the board **landscape** (long edge horizontal).

RST is hardware reset only. BOOT (GPIO0) is for flashing.

## Setup

1. Copy WiFi and weather credentials:

```powershell
copy firmware\secrets.example.h firmware\secrets.h
# Edit secrets.h — WIFI_SSID, WIFI_PASS, WEATHER_API_KEY, WEATHER_CITY
```

2. Generate art, compile pack, flash:

```powershell
uv run python scripts/generate_eyes_art.py
python -c "from pathlib import Path; from nomabot.assets.compiler import compile_pack; compile_pack(Path('assets/characters/eyes'), Path('compiled/eyes'), 'lilygo_tdisplay_s3_landscape')"
python scripts/copy_pack_to_firmware_data.py
cd firmware
pio run -e lilygo_tdisplay_s3 -t upload -t uploadfs --upload-port COM3
```

3. Power from USB — device runs alone. Optional serial monitor at 115200 for debug logs.

## Do NOT use nomabot_desktop for eyes

The old desktop app was for the full-body nomabot robot. Eyes firmware is standalone. If you connect USB while desktop is open, it will detect eyes mode and **not** send control commands.

For the portrait nomabot robot, use the `main` branch and `nomabot_desktop` there.

## Tests

```powershell
uv run pytest sdk/tests/test_eyes_render_mode.py -q
```
