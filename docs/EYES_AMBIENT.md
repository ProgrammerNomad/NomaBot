# Eyes Ambient Branch

Landscape **eyes-only** desk pet inspired by [Desper](https://www.youtube.com/watch?v=DZxC5BGkKT8). Branch: `feature/eyes-ambient`.

## Layout

- **320×170 landscape** (rotated T-Display S3)
- Horizontal eye pair sprites (`eyes_neutral`, `eyes_blink`, …)
- Top ambient bar: clock (left) + weather (right)
- Weather fetched on **desktop** via OpenWeatherMap

## Setup

1. Generate art and compile pack:

```powershell
uv run python scripts/generate_eyes_art.py
uv run nomabot build-assets --input assets/characters/eyes --output compiled/eyes --profile lilygo_tdisplay_s3_landscape
uv run python scripts/copy_pack_to_firmware_data.py --compiled compiled/eyes --character-id eyes
```

2. Flash firmware + LittleFS:

```powershell
cd firmware
pio run -e lilygo_tdisplay_s3 -t upload -t uploadfs --upload-port COM3
```

3. Run desktop (loads `eyes` character by default on this branch):

```powershell
uv run python -m nomabot_desktop --port COM3 --dev
```

**Hold the board landscape** — long edge horizontal, USB on the side — so the two eyes sit side-by-side. In portrait hold the eyes stack vertically (firmware uses rotation 1 for 320×170).

If you see `PermissionError: Access is denied` on COM3, close serial monitors first:

```powershell
Get-Process python*, pio* -ErrorAction SilentlyContinue | Stop-Process -Force
```

Then retry with a single desktop instance.

## OpenWeatherMap (optional)

Set in desktop settings storage (SQLite `settings` table) or via config API:

- `weather_api_key` — from [openweathermap.org](https://openweathermap.org/api)
- `weather_city` — e.g. `Mumbai,IN`
- `weather_enabled` — `true`

Without a key, eyes + clock still work; weather bar stays empty until configured.

## Switch characters

- **Eyes (landscape):** `load_character` → `eyes`
- **NomaBot (portrait):** `load_character` → `nomabot`

Portrait restores rotation 0; eyes sets rotation 1.

## Tests

```powershell
uv run pytest sdk/tests/test_eyes_render_mode.py sdk/tests/test_weather_command.py -q
```
