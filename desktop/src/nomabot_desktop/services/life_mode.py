"""Life mode service — desktop world context from time of day."""

from __future__ import annotations

import logging
from datetime import datetime

from PySide6.QtCore import QTimer

from nomabot.types import Priority
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_source import CommandSource
from nomabot_desktop.core.events import StateRequest

logger = logging.getLogger("noma.life_mode")

LIFE_MODES = ("work", "home", "travel", "night", "vacation")


def life_mode_for_time(now: datetime | None = None) -> str:
    now = now or datetime.now()
    hour = now.hour
    if 22 <= hour or hour < 6:
        return "night"
    if now.weekday() >= 5:
        return "vacation"
    if 9 <= hour < 18:
        return "work"
    if 18 <= hour < 22:
        return "home"
    return "home"


class LifeModeService:
    def __init__(self, bus: EventBus) -> None:
        self._bus = bus
        self._current = "work"
        self._timer = QTimer()
        self._timer.timeout.connect(self._tick)

    def start(self) -> None:
        self._tick()
        self._timer.start(60_000)
        logger.info("LifeModeService started")

    def stop(self) -> None:
        self._timer.stop()

    def _tick(self) -> None:
        mode = life_mode_for_time()
        if mode == self._current:
            return
        self._current = mode
        logger.info("Life mode -> %s", mode)
        self._bus.publish(
            "state.request",
            StateRequest(
                state="idle" if mode in {"night", "vacation"} else self._current,
                priority=Priority.LOW,
                source=CommandSource.LIFE_MODE,
                life_mode=mode,
                activity="sleep" if mode == "night" else None,
            ),
        )
