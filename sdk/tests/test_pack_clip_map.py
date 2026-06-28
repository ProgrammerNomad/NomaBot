"""Pack clip map tests — behavior.json clip resolution."""

from __future__ import annotations

import json
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
BEHAVIOR_JSON = ROOT / "compiled" / "nomabot" / "behavior.json"


def _load_clip_map(data: dict) -> dict[str, str]:
    """Mirror firmware PackClipMap merge order."""
    clips: dict[str, str] = {}

    def add_entries(behaviors: list[dict]) -> None:
        for entry in behaviors:
            behavior_id = entry.get("id")
            clip = entry.get("clip")
            if behavior_id and clip and behavior_id not in clips:
                clips[behavior_id] = clip

    for activity in (data.get("activities") or {}).values():
        add_entries(list(activity.get("behaviors") or []))

    for emotion_table in (data.get("emotions") or {}).values():
        for activity in emotion_table.values():
            add_entries(list(activity.get("behaviors") or []))

    for behavior_id, clip in (data.get("behavior_clips") or {}).items():
        clips[behavior_id] = clip

    return clips


@pytest.fixture
def clip_map() -> dict[str, str]:
    if not BEHAVIOR_JSON.exists():
        pytest.skip("compiled behavior.json missing — run nomabot build-assets")
    data = json.loads(BEHAVIOR_JSON.read_text(encoding="utf-8"))
    return _load_clip_map(data)


def test_look_around_maps_to_look_left(clip_map: dict[str, str]) -> None:
    assert clip_map["look_around"] == "look_left"


def test_typing_maps_to_coding(clip_map: dict[str, str]) -> None:
    assert clip_map["typing"] == "coding"


def test_think_maps_to_think(clip_map: dict[str, str]) -> None:
    assert clip_map["think"] == "think"


def test_blink_not_stuck_on_idle(clip_map: dict[str, str]) -> None:
    assert clip_map["blink"] == "blink"


def test_behavior_clips_override(clip_map: dict[str, str]) -> None:
    assert clip_map["breathing"] == "idle"
