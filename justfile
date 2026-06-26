# NomaBot task runner - install: https://github.com/casey/just

# Sync dependencies
sync:
    uv sync

# Run all tests
test:
    uv run pytest sdk/tests desktop/tests

# Lint
lint:
    uv run ruff check sdk desktop
    uv run ruff format --check sdk desktop

# Format
format:
    uv run ruff format sdk desktop

# Protocol fixture lint
protocol:
    uv run nomabot protocol lint sdk/tests/fixtures/protocol

# Validate device profiles
profiles:
    uv run python scripts/validate_profiles.py

# Build firmware
firmware:
    cd firmware && pio run -e lilygo_tdisplay_s3

# Run desktop app
desktop:
    uv run python -m nomabot_desktop

# Run with emulator window
emulator:
    uv run python -m nomabot_desktop --emulator

# Mock device tests only
mock:
    uv run pytest sdk/tests -k mock

# Asset compiler (stub / full)
build-assets *ARGS:
    uv run nomabot build-assets {{ARGS}}

# Generate sprites, compile pack, copy to firmware LittleFS data
assets:
    uv run python scripts/generate_placeholder_sprites.py
    uv run nomabot build-assets --input assets/characters/nomabot --output compiled/nomabot --profile lilygo_tdisplay_s3
    uv run python scripts/copy_pack_to_firmware_data.py

# Flash firmware + LittleFS filesystem
flash-all:
    just assets
    cd firmware && pio run -e lilygo_tdisplay_s3 -t upload && pio run -e lilygo_tdisplay_s3 -t uploadfs

# Full CI check locally
ci: lint test protocol profiles
