"""Mock device tests."""

import pytest

from nomabot.client import NomaBot, NomaClient
from nomabot.protocol.commands import HelloParams, build_command
from nomabot.testing import MockDevice


@pytest.mark.asyncio
async def test_mock_hello():
    mock = MockDevice()
    client = NomaClient(mock)
    await mock.connect()
    resp = await client.send_command(build_command("hello", HelloParams()))
    assert resp.ok is True
    assert resp.data["device_id"] == "mock-esp32"


@pytest.mark.asyncio
async def test_mock_play_animation():
    mock = MockDevice()
    client = NomaClient(mock)
    await mock.connect()
    await client.send_command(build_command("hello", HelloParams()))
    await client.play_animation("idle")
    assert mock.last_animation == "idle"


@pytest.mark.asyncio
async def test_noma_bot_say():
    mock = MockDevice()
    bot = NomaBot.from_mock(mock)
    await mock.connect()
    await bot._client.send_command(build_command("hello", HelloParams()))
    await bot.say("Hello")
    assert mock.last_message == "Hello"
