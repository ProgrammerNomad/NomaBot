"""Overlay service — speech/toasts bypass world context."""

from __future__ import annotations

import logging
from collections.abc import Callable, Coroutine
from typing import Any

from nomabot.protocol.commands import ShowMessageParams, build_command
from nomabot.types import Priority
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_dispatcher import CommandDispatcher
from nomabot_desktop.core.events import OverlayShow
from nomabot_desktop.core.priority_queue import PriorityQueue

logger = logging.getLogger("noma.overlay")


class OverlayService:
    """Routes show_message to device; never touches StateManager or Brain context."""

    def __init__(
        self,
        bus: EventBus,
        queue: PriorityQueue,
        dispatcher: CommandDispatcher,
        schedule: Callable[[Coroutine[Any, Any, Any]], None],
    ) -> None:
        self._bus = bus
        self._queue = queue
        self._dispatcher = dispatcher
        self._schedule = schedule
        bus.subscribe("overlay.show", self._on_overlay_show)

    def show(
        self,
        *,
        overlay_id: str,
        text: str,
        priority: Priority = Priority.NORMAL,
        duration_ms: int = 5000,
        style: str = "speech",
        device_id: str | None = None,
    ) -> None:
        self._schedule(
            self._dispatch(
                overlay_id=overlay_id,
                text=text,
                priority=priority,
                duration_ms=duration_ms,
                style=style,
                device_id=device_id,
            )
        )

    def _on_overlay_show(self, payload: OverlayShow) -> None:
        self.show(
            overlay_id=payload.overlay_id,
            text=payload.text,
            priority=payload.priority,
            duration_ms=payload.duration_ms,
            style=payload.style,
            device_id=payload.device_id,
        )

    async def _dispatch(
        self,
        *,
        overlay_id: str,
        text: str,
        priority: Priority,
        duration_ms: int,
        style: str,
        device_id: str | None,
    ) -> None:
        cmd = build_command(
            "show_message",
            ShowMessageParams(
                id=overlay_id,
                text=text,
                style=style,
                duration_ms=duration_ms,
                priority=int(priority),
            ),
        )
        self._queue.enqueue(cmd, priority=priority, device_id=device_id)
        await self._dispatcher.flush()
        logger.info("Overlay %s: %s", overlay_id, text[:40])
        self._bus.publish(
            "overlay.changed",
            OverlayShow(
                overlay_id=overlay_id,
                text=text,
                priority=priority,
                duration_ms=duration_ms,
                style=style,
                device_id=device_id,
            ),
        )
