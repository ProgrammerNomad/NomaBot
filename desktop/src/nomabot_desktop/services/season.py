"""Season context from calendar date."""

from __future__ import annotations

import logging
from datetime import datetime

from PySide6.QtCore import QTimer

from nomabot.types import Priority
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_source import CommandSource
from nomabot_desktop.core.events import StateRequest

logger = logging.getLogger("noma.season")


def season_for_date(now: datetime | None = None) -> str:
    now = now or datetime.now()
    month = now.month
    if month in (12, 1, 2):
        return "winter"
    if month in (3, 4, 5):
        return "spring"
    if month in (6, 7, 8, 9):
        return "monsoon" if month in (6, 7, 8) else "summer"
    return "festival" if month == 10 else "winter"


class SeasonService:
    def __init__(self, bus: EventBus) -> None:
        self._bus = bus
        self._current = ""
        self._timer = QTimer()
        self._timer.timeout.connect(self._tick)

    def start(self) -> None:
        self._tick()
        self._timer.start(3_600_000)
        logger.info("SeasonService started")

    def stop(self) -> None:
        self._timer.stop()

    def _tick(self) -> None:
        season = season_for_date()
        if season == self._current:
            return
        self._current = season
        logger.info("Season -> %s", season)
        self._bus.publish(
            "state.request",
            StateRequest(
                state="idle",
                priority=Priority.LOW,
                source=CommandSource.SYSTEM,
                season=season,
            ),
        )
