"""Runtime priority merge tests."""

import pytest

from nomabot.testing import MockDevice
from nomabot.types import Priority, RenderRequest
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_dispatcher import CommandDispatcher
from nomabot_desktop.core.priority_queue import PriorityQueue
from nomabot_desktop.core.runtime import NomaRuntime
from nomabot_desktop.transport.manager import TransportManager


async def _make_runtime() -> tuple[NomaRuntime, MockDevice, TransportManager]:
    bus = EventBus()
    tm = TransportManager(bus)
    mock = MockDevice()
    tm.register("d1", "mock", {}, inner=mock)
    await tm.connect("d1")
    queue = PriorityQueue()
    dispatcher = CommandDispatcher(queue, tm)
    dispatcher.set_default_device("d1")
    runtime = NomaRuntime(queue, dispatcher)
    return runtime, mock, tm


@pytest.mark.asyncio
async def test_priority_merge_animation():
    runtime, mock, tm = await _make_runtime()

    await runtime.submit_renderer(RenderRequest(animation="idle", priority=Priority.LOW))
    await runtime.submit_renderer(RenderRequest(animation="coding", priority=Priority.HIGH))

    assert mock.last_animation == "coding"
    await tm.disconnect_all()


@pytest.mark.asyncio
async def test_low_priority_does_not_override_high():
    runtime, mock, tm = await _make_runtime()

    await runtime.submit_renderer(RenderRequest(animation="coding", priority=Priority.HIGH))
    await runtime.submit_renderer(RenderRequest(animation="idle", priority=Priority.LOW))

    assert mock.last_animation == "coding"
    await tm.disconnect_all()


@pytest.mark.asyncio
async def test_context_submit_does_not_send_animation():
    runtime, mock, tm = await _make_runtime()

    await runtime.submit(
        RenderRequest(activity="coding", animation="idle", priority=Priority.NORMAL)
    )

    assert mock.last_activity == "coding"
    assert mock.last_animation is None
    await tm.disconnect_all()
