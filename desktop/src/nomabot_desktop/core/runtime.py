"""Noma Runtime - context commands only (no overlays)."""

from __future__ import annotations

import logging
from collections.abc import Callable
from dataclasses import dataclass

from nomabot.protocol.commands import (
    PlayAnimationParams,
    SetActivityParams,
    SetBackgroundParams,
    SetEmotionParams,
    SetLifeModeParams,
    SetSeasonParams,
    TriggerHabitParams,
    build_command,
)
from nomabot.protocol.envelope import Envelope
from nomabot.types import Priority, RenderRequest
from nomabot_desktop.core.asset_registry import AssetRegistry
from nomabot_desktop.core.command_dispatcher import CommandDispatcher
from nomabot_desktop.core.priority_queue import PriorityQueue

logger = logging.getLogger("noma.runtime")

ACTIVITY_STATES = frozenset({"idle", "coding", "sleep", "gaming", "travel"})


@dataclass
class _MergedState:
    animation: str | None = None
    background: str | None = None
    activity: str | None = None
    emotion: str | None = None
    life_mode: str | None = None
    habit: str | None = None
    season: str | None = None
    priority: Priority = Priority.BACKGROUND


class NomaRuntime:
    """Builds context protocol commands; overlays use OverlayService."""

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
        """Context commands only — arbitrator already filtered sources."""
        commands: list[Envelope] = []
        s = self._state

        if request.activity:
            s.activity = request.activity
            commands.append(
                build_command("set_activity", SetActivityParams(activity=request.activity))
            )

        if request.emotion:
            s.emotion = request.emotion
            commands.append(build_command("set_emotion", SetEmotionParams(emotion=request.emotion)))

        if request.life_mode:
            s.life_mode = request.life_mode
            commands.append(
                build_command("set_life_mode", SetLifeModeParams(mode=request.life_mode))
            )

        if request.habit:
            commands.append(build_command("trigger_habit", TriggerHabitParams(habit=request.habit)))

        if request.season:
            commands.append(build_command("set_season", SetSeasonParams(season=request.season)))

        self._notify()

        for cmd in commands:
            self._queue.enqueue(cmd, priority=request.priority, device_id=request.device_id)
            logger.info("Runtime queued %s", cmd.cmd)

        await self._dispatcher.flush()
        return commands

    async def submit_renderer(self, request: RenderRequest) -> list[Envelope]:
        """Renderer-only commands: animation and background."""
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

        self._notify()

        for cmd in commands:
            self._queue.enqueue(cmd, priority=request.priority, device_id=request.device_id)
            logger.info("Runtime queued %s", cmd.cmd)

        await self._dispatcher.flush()
        return commands
