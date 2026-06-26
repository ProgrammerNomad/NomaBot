"""Protocol tests."""

import json
from pathlib import Path

import pytest

from nomabot.protocol.commands import PlayAnimationParams, build_command
from nomabot.protocol.envelope import MessageType, parse_line, to_json_line


def test_envelope_roundtrip():
    env = build_command("ping")
    line = to_json_line(env)
    parsed = parse_line(line.strip())
    assert parsed.v == 1
    assert parsed.type == MessageType.COMMAND
    assert parsed.cmd == "ping"


def test_play_animation_params():
    env = build_command("play_animation", PlayAnimationParams(animation="idle", loop=True))
    assert env.params["animation"] == "idle"
    assert env.params["loop"] is True


def test_missing_version_raises():
    data = json.loads(
        (Path(__file__).parent / "fixtures/protocol/invalid/missing_version.json").read_text()
    )
    with pytest.raises(ValueError, match="v"):
        parse_line(json.dumps(data, separators=(",", ":")))


def test_fixture_ping():
    data = json.loads((Path(__file__).parent / "fixtures/protocol/ping.json").read_text())
    env = parse_line(json.dumps(data, separators=(",", ":")))
    assert env.cmd == "ping"
