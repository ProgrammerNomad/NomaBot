"""load_character and diagnostics protocol tests."""

import pytest

from nomabot.client import NomaClient
from nomabot.protocol.commands import HelloParams, build_command
from nomabot.testing import MockDevice


@pytest.mark.asyncio
async def test_mock_load_character():
    mock = MockDevice()
    client = NomaClient(mock)
    await mock.connect()
    await client.send_command(build_command("hello", HelloParams()))
    resp = await client.load_character("nomabot")
    assert resp.ok is True
    assert resp.data["pack_id"] == "nomabot"
    assert resp.data["uuid"] == mock.pack_uuid
    assert resp.data["version"]["major"] == 0
    assert mock.last_character_id == "nomabot"


@pytest.mark.asyncio
async def test_mock_diagnostics():
    mock = MockDevice()
    client = NomaClient(mock)
    await mock.connect()
    await client.send_command(build_command("hello", HelloParams()))
    await client.play_animation("coding")
    resp = await client.diagnostics()
    assert resp.ok is True
    assert resp.data["animation"] == "coding"
    assert "heap_free" in resp.data
    assert "frame" in resp.data
