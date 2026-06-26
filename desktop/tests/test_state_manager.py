"""State manager tests."""

import asyncio

import pytest

from nomabot.testing import MockDevice
from nomabot.types import Priority
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_dispatcher import CommandDispatcher
from nomabot_desktop.core.events import StateRequest
from nomabot_desktop.core.priority_queue import PriorityQueue
from nomabot_desktop.core.runtime import NomaRuntime
from nomabot_desktop.core.state_manager import StateManager
from nomabot_desktop.transport.manager import TransportManager


@pytest.mark.asyncio
async def test_state_manager_requests_runtime() -> None:
    bus = EventBus()
    tm = TransportManager(bus)
    mock = MockDevice()
    tm.register("d1", "mock", {}, inner=mock)
    await tm.connect("d1")
    queue = PriorityQueue()
    dispatcher = CommandDispatcher(queue, tm)
    dispatcher.set_default_device("d1")
    runtime = NomaRuntime(queue, dispatcher)

    pending: list[asyncio.Future] = []

    def schedule(coro):
        pending.append(asyncio.ensure_future(coro))

    sm = StateManager(bus, schedule)
    sm.bind_runtime(runtime.submit)

    sm.request(StateRequest(state="coding", priority=Priority.NORMAL, animation="coding"))
    if pending:
        await pending[0]

    assert mock.last_animation == "coding"
    await tm.disconnect_all()
