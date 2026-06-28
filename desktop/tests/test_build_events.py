"""Build events and scheduler action tests."""

import asyncio

import pytest

from nomabot.testing import MockDevice
from nomabot.types import Priority
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_dispatcher import CommandDispatcher
from nomabot_desktop.core.events import SchedulerFire, StateRequest
from nomabot_desktop.core.priority_queue import PriorityQueue
from nomabot_desktop.core.runtime import NomaRuntime
from nomabot_desktop.core.state_manager import StateManager
from nomabot_desktop.services.build_events import BuildEventService, BuildResult
from nomabot_desktop.transport.manager import TransportManager


@pytest.mark.asyncio
async def test_build_fail_sets_frustrated_emotion() -> None:
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
    BuildEventService(bus)

    bus.publish("build.result", BuildResult(success=False, message="Build failed"))
    if pending:
        await pending[0]

    assert mock.last_emotion == "frustrated"
    assert mock.last_message == "Build failed"
    await tm.disconnect_all()


@pytest.mark.asyncio
async def test_scheduler_trigger_habit() -> None:
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

    bus.publish(
        "scheduler.fire",
        SchedulerFire(
            job_id="morning",
            action="TriggerHabit",
            parameters={"habit": "morning"},
            priority=Priority.NORMAL,
        ),
    )
    if pending:
        await pending[0]

    cmds = [e.cmd for e in mock.sent_envelopes if e.cmd == "trigger_habit"]
    assert cmds
    await tm.disconnect_all()
