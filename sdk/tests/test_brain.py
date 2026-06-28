"""Unified Brain tests."""

from pathlib import Path

import pytest

from nomabot.brain import Brain


BEHAVIOR_YAML = Path(__file__).resolve().parents[2] / "assets" / "characters" / "nomabot" / "behavior.yaml"


@pytest.fixture
def brain() -> Brain:
    b = Brain.from_yaml(BEHAVIOR_YAML)
    b.personality.coffee_love = 90
    b.runtime.coffee_love = 90
    b.energy = 80
    return b


def test_goal_progress_on_coding(brain: Brain) -> None:
    brain.set_activity("coding")
    assert brain.goal == "focus"
    brain.behavior_id = "typing"
    brain._update_goal()
    assert brain.goal_progress == 8


def test_frustrated_starts_recover_sequence(brain: Brain) -> None:
    brain.set_activity("coding")
    brain.set_emotion("frustrated")
    assert brain.goal == "recover"
    assert brain._pick_mode == "sequence"
    assert brain.behavior_id == "think"


def test_trigger_morning_habit(brain: Brain) -> None:
    brain.trigger_habit("morning")
    assert brain._pick_mode == "sequence"
    assert brain.behavior_id == "stretch"


def test_curiosity_flag(brain: Brain) -> None:
    brain._last_curiosity = 0
    brain._update_curiosity(100.0)
    assert brain.curiosity_active or brain.behavior_label


def test_diagnostics_payload(brain: Brain) -> None:
    diag = brain.diagnostics()
    assert "energy" in diag
    assert "goal" in diag
    assert diag["render_mode"] == "text"
