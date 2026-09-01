"""Firmware render_mode contract - behavior.json must fit parse buffer."""

from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
BEHAVIOR_JSON = ROOT / "compiled" / "nomabot" / "behavior.json"

# character_runtime.cpp StaticJsonDocument size (must fit full behavior.json).
FIRMWARE_BEHAVIOR_JSON_CAPACITY = 8192


def test_behavior_json_fits_firmware_buffer() -> None:
    if not BEHAVIOR_JSON.exists():
        pytest.skip("compiled behavior.json missing - run nomabot build-assets")
    size = BEHAVIOR_JSON.stat().st_size
    assert size < FIRMWARE_BEHAVIOR_JSON_CAPACITY, (
        f"behavior.json is {size}B; firmware buffer is {FIRMWARE_BEHAVIOR_JSON_CAPACITY}B"
    )


def test_behavior_json_sprite_mode() -> None:
    if not BEHAVIOR_JSON.exists():
        pytest.skip("compiled behavior.json missing - run nomabot build-assets")
    import json

    data = json.loads(BEHAVIOR_JSON.read_text(encoding="utf-8"))
    assert data.get("render_mode") == "sprite"
