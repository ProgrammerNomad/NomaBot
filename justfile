# Eyes Ambient task runner - install: https://github.com/casey/just

sync:
    uv sync --all-packages

test:
    uv run pytest sdk/tests -q

lint:
    uv run ruff check sdk scripts
    uv run ruff format --check sdk scripts

format:
    uv run ruff format sdk scripts

profiles:
    uv run python scripts/validate_profiles.py

assets:
    uv run python scripts/generate_eyes_art.py
    uv run python -c "from pathlib import Path; from nomabot.assets.compiler import compile_pack; compile_pack(Path('assets/characters/eyes'), Path('compiled/eyes'), 'lilygo_tdisplay_s3_landscape')"
    uv run python scripts/copy_pack_to_firmware_data.py

firmware:
    cd firmware && pio run -e lilygo_tdisplay_s3

flash:
    just assets
    cd firmware && pio run -e lilygo_tdisplay_s3 -t upload -t uploadfs

ci: lint test profiles assets firmware
