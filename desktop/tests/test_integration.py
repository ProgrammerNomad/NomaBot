"""Integration: runtime + mock device."""

import pytest

from nomabot.testing import MockDevice
from nomabot.types import Priority, RenderRequest
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_dispatcher import CommandDispatcher
from nomabot_desktop.core.priority_queue import PriorityQueue
from nomabot_desktop.core.runtime import NomaRuntime
from nomabot_desktop.transport.manager import TransportManager


@pytest.mark.asyncio
async def test_runtime_to_mock_play_animation():
    bus = EventBus()
    tm = TransportManager(bus)
    mock = MockDevice()
    tm.register("d1", "mock", {}, inner=mock)
    await tm.connect("d1")
    queue = PriorityQueue()
    dispatcher = CommandDispatcher(queue, tm)
    dispatcher.set_default_device("d1")
    runtime = NomaRuntime(queue, dispatcher)

    await runtime.submit_renderer(RenderRequest(animation="coding", priority=Priority.NORMAL))

    assert mock.last_animation == "coding"
    assert len(mock.sent_envelopes) >= 2
    assert any(e.cmd == "play_animation" for e in mock.sent_envelopes)
    await tm.disconnect_all()
