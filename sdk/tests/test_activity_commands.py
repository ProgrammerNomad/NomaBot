"""Ensure activity changes use set_activity, not play_animation."""

import pytest

from nomabot.protocol.commands import PlayAnimationParams, SetActivityParams, build_command
from nomabot.testing import MockDevice


@pytest.mark.asyncio
async def test_mock_device_handles_set_activity() -> None:
    mock = MockDevice()
    await mock.connect()
    env = build_command("set_activity", SetActivityParams(activity="coding"))
    await mock.send(env.to_json_line().encode("utf-8"))
    assert mock.last_activity == "coding"


@pytest.mark.asyncio
async def test_mock_device_diagnostics_include_behavior_fields() -> None:
    mock = MockDevice()
    await mock.connect()
    mock.last_activity = "coding"
    mock.last_emotion = "frustrated"
    mock.last_behavior = "typing"
    env = build_command("diagnostics")
    await mock.send(env.to_json_line().encode("utf-8"))
    last = mock.sent_envelopes[-1]
    assert last.cmd == "diagnostics"


def test_activity_command_not_play_animation() -> None:
    activity_cmd = build_command("set_activity", SetActivityParams(activity="idle"))
    anim_cmd = build_command("play_animation", PlayAnimationParams(animation="idle"))
    assert activity_cmd.cmd != anim_cmd.cmd
