"""Behavior compiler golden tests."""

from pathlib import Path

import pytest

from nomabot.assets.behavior_compiler import compile_behavior, write_behavior_json


ROOT = Path(__file__).resolve().parents[2]
NOMABOT = ROOT / "assets" / "characters" / "nomabot"


def test_compile_behavior_includes_personality_and_likes() -> None:
    data = compile_behavior(NOMABOT)
    assert data["render_mode"] == "text"
    assert "personality" in data
    assert data["personality"]["coffee_love"] == 90
    assert "coffee" in data["likes"]
    assert "habits" in data
    assert "goals" in data


def test_write_behavior_json(tmp_path: Path) -> None:
    out = write_behavior_json(NOMABOT, tmp_path)
    assert out.exists()
    assert out.name == "behavior.json"
    text = out.read_text(encoding="utf-8")
    assert "life_modes" in text
    assert "emotion_meta" in text


def test_compile_behavior_missing_file(tmp_path: Path) -> None:
    with pytest.raises(FileNotFoundError):
        compile_behavior(tmp_path)
