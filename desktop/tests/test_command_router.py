"""CommandRouter and overlay routing tests."""

from __future__ import annotations

import asyncio

import pytest

from nomabot.testing import MockDevice
from nomabot.types import Priority, RenderRequest
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_dispatcher import CommandDispatcher
from nomabot_desktop.core.command_router import CommandRouter
from nomabot_desktop.core.command_source import CommandSource
from nomabot_desktop.core.events import OverlayShow, StateRequest
from nomabot_desktop.core.overlay_service import OverlayService
from nomabot_desktop.core.priority_queue import PriorityQueue
from nomabot_desktop.core.runtime import NomaRuntime
from nomabot_desktop.core.state_manager import StateManager
from nomabot_desktop.transport.manager import TransportManager


async def _setup() -> tuple[
    EventBus,
    StateManager,
    OverlayService,
    CommandRouter,
    MockDevice,
    TransportManager,
    list,
]:
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
    sm.bind_runtime(runtime.submit, runtime.submit_renderer)
    overlay = OverlayService(bus, queue, dispatcher, schedule)
    router = CommandRouter(sm, overlay, runtime, schedule)
    return bus, sm, overlay, router, mock, tm, pending


@pytest.mark.asyncio
async def test_overlay_bypasses_state_manager_after_high_context() -> None:
    _, sm, _, router, mock, tm, pending = await _setup()

    sm.request(
        StateRequest(
            state="coding",
            priority=Priority.HIGH,
            source=CommandSource.SYSTEM,
            emotion="frustrated",
        )
    )
    if pending:
        await pending.pop(0)

    router.overlay(overlay_id="dev_hello", text="Hello", priority=Priority.NORMAL)
    if pending:
        await pending.pop(0)

    assert mock.last_emotion == "frustrated"
    assert mock.last_message == "Hello"
    show_cmds = [e for e in mock.sent_envelopes if e.cmd == "show_message"]
    assert show_cmds[-1].params.get("id") == "dev_hello"
    await tm.disconnect_all()


@pytest.mark.asyncio
async def test_context_does_not_call_show_message() -> None:
    _, _, _, router, mock, tm, pending = await _setup()

    router.context(
        StateRequest(state="coding", priority=Priority.NORMAL, source=CommandSource.DEV_PANEL)
    )
    if pending:
        await pending.pop(0)

    show_cmds = [e for e in mock.sent_envelopes if e.cmd == "show_message"]
    assert not show_cmds
    assert mock.last_activity == "coding"
    await tm.disconnect_all()


@pytest.mark.asyncio
async def test_renderer_route_uses_submit_renderer() -> None:
    bus, sm, overlay, router, mock, tm, pending = await _setup()

    router.renderer(RenderRequest(animation="coding", priority=Priority.HIGH))
    if pending:
        await pending.pop(0)

    assert mock.last_animation == "coding"
    await tm.disconnect_all()


@pytest.mark.asyncio
async def test_overlay_show_event_dispatched() -> None:
    bus, _, overlay, _, mock, tm, pending = await _setup()
    overlay  # keep reference

    bus.publish(
        "overlay.show",
        OverlayShow(overlay_id="build_ok", text="Build OK", priority=Priority.HIGH),
    )
    if pending:
        await pending.pop(0)

    assert mock.last_message == "Build OK"
    await tm.disconnect_all()
