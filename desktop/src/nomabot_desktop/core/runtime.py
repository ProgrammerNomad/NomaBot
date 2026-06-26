"""Noma Runtime - central render orchestrator."""

from __future__ import annotations

import logging
from collections.abc import Callable
from dataclasses import dataclass

from nomabot.protocol.commands import (
    PlayAnimationParams,
    SetBackgroundParams,
    SetStateParams,
    ShowMessageParams,
    build_command,
)
from nomabot.protocol.envelope import Envelope
from nomabot.types import Priority, RenderRequest
from nomabot_desktop.core.asset_registry import AssetRegistry
from nomabot_desktop.core.command_dispatcher import CommandDispatcher
from nomabot_desktop.core.priority_queue import PriorityQueue

logger = logging.getLogger("noma.runtime")


@dataclass
class _MergedState:
    animation: str | None = None
    background: str | None = None
    message_text: str | None = None
    message_style: str = "speech"
    state: str | None = None
    priority: Priority = Priority.BACKGROUND


class NomaRuntime:
    """Builds protocol commands and dispatches via priority queue."""

    def __init__(
        self,
        queue: PriorityQueue,
        dispatcher: CommandDispatcher,
        assets: AssetRegistry | None = None,
    ) -> None:
        self._queue = queue
        self._dispatcher = dispatcher
        self._assets = assets or AssetRegistry()
        self._state = _MergedState()
        self._listeners: list[Callable[[_MergedState], None]] = []

    @property
    def assets(self) -> AssetRegistry:
        return self._assets

    def on_state_changed(self, callback: Callable[[_MergedState], None]) -> None:
        self._listeners.append(callback)

    def _notify(self) -> None:
        for cb in self._listeners:
            cb(self._state)

    async def submit(self, request: RenderRequest) -> list[Envelope]:
        """Merge by priority, enqueue commands, flush dispatcher."""
        commands: list[Envelope] = []
        s = self._state

        if request.animation and request.priority >= s.priority:
            s.animation = request.animation
            s.priority = request.priority
            commands.append(
                build_command("play_animation", PlayAnimationParams(animation=request.animation))
            )

        if request.background and request.priority >= s.priority:
            s.background = request.background
            commands.append(
                build_command("set_background", SetBackgroundParams(background=request.background))
            )

        if request.message and request.priority >= s.priority:
            s.message_text = request.message.text
            s.message_style = request.message.style
            commands.append(
                build_command(
                    "show_message",
                    ShowMessageParams(
                        text=request.message.text,
                        style=request.message.style,
                        duration_ms=request.message.duration_ms,
                    ),
                )
            )

        if request.state:
            s.state = request.state
            commands.append(build_command("set_state", SetStateParams(state=request.state)))

        self._notify()

        for cmd in commands:
            self._queue.enqueue(cmd, priority=request.priority, device_id=request.device_id)
            logger.info("Runtime queued %s", cmd.cmd)

        await self._dispatcher.flush()
        return commands
