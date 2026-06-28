"""Behavior engine tests."""

from pathlib import Path

import pytest

from nomabot.behavior import BehaviorEngine, Personality


BEHAVIOR_YAML = Path(__file__).resolve().parents[2] / "assets" / "characters" / "nomabot" / "behavior.yaml"


@pytest.fixture
def engine() -> BehaviorEngine:
    eng = BehaviorEngine.from_yaml(BEHAVIOR_YAML)
    eng.personality = Personality(coffee_love=90, optimism=90, curiosity=70, sleepiness=20, playfulness=60)
    eng.runtime.coffee_love = 90
    eng.runtime.energy = 80
    return eng


def test_activity_change_picks_coding_behavior(engine: BehaviorEngine) -> None:
    engine.set_activity("coding")
    assert engine.behavior_id in {"typing", "think"}


def test_frustrated_emotion_overrides_coding_table(engine: BehaviorEngine) -> None:
    engine.set_activity("coding")
    engine.set_emotion("frustrated")
    assert engine.behavior_id in {"think", "typing_slow"}


def test_sleepy_idle_prefers_blink(engine: BehaviorEngine) -> None:
    engine.set_activity("idle")
    engine.set_emotion("sleepy")
    picks = {engine.pick_behavior(force=True) for _ in range(30)}
    assert picks <= {"blink_slow", "breathing"}


def test_set_activity_command_in_runtime_path() -> None:
    from nomabot.protocol.commands import SetActivityParams, build_command

    env = build_command("set_activity", SetActivityParams(activity="coding"))
    assert env.cmd == "set_activity"
    assert env.params["activity"] == "coding"
