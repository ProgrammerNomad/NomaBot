"""Behavior compiler tests."""

from pathlib import Path

import pytest

from nomabot.assets.behavior_compiler import compile_behavior, write_behavior_json

ROOT = Path(__file__).resolve().parents[2]
EYES = ROOT / "assets" / "characters" / "eyes"


def test_compile_behavior_eyes_mode() -> None:
    data = compile_behavior(EYES)
    assert data["render_mode"] == "eyes"
    assert "activities" in data
    idle_behaviors = data["activities"]["idle"]["behaviors"]
    behavior_ids = {entry["id"] for entry in idle_behaviors}
    assert "idle" in behavior_ids
    assert "blink" in behavior_ids


def test_write_behavior_json(tmp_path: Path) -> None:
    out = write_behavior_json(EYES, tmp_path)
    assert out.exists()
    assert out.name == "behavior.json"
    text = out.read_text(encoding="utf-8")
    assert "render_mode" in text
    assert "behavior_clips" in text


def test_compile_behavior_missing_file(tmp_path: Path) -> None:
    with pytest.raises(FileNotFoundError):
        compile_behavior(tmp_path)
