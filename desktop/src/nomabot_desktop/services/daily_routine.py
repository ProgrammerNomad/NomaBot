"""Daily routine — trigger habits at scheduled times."""

from __future__ import annotations

import logging
from datetime import datetime

from PySide6.QtCore import QTimer

from nomabot.types import Priority
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_source import CommandSource
from nomabot_desktop.core.events import StateRequest

logger = logging.getLogger("noma.daily_routine")

ROUTINE: list[tuple[int, int, str, dict]] = [
    (8, 0, "morning", {"habit": "morning", "life_mode": "work"}),
    (10, 30, "focus_block", {"activity": "coding", "life_mode": "work"}),
    (13, 0, "tea_break", {"habit": "tea_break"}),
    (18, 30, "evening", {"habit": "evening", "life_mode": "home"}),
    (22, 30, "wind_down", {"life_mode": "night", "activity": "sleep", "habit": "dream"}),
]


class DailyRoutineService:
    def __init__(self, bus: EventBus) -> None:
        self._bus = bus
        self._fired_today: set[str] = set()
        self._last_day = datetime.now().date()
        self._timer = QTimer()
        self._timer.timeout.connect(self._tick)

    def start(self) -> None:
        self._timer.start(30_000)
        logger.info("DailyRoutineService started")

    def stop(self) -> None:
        self._timer.stop()

    def _tick(self) -> None:
        now = datetime.now()
        if now.date() != self._last_day:
            self._fired_today.clear()
            self._last_day = now.date()
        for hour, minute, job_id, params in ROUTINE:
            if job_id in self._fired_today:
                continue
            if now.hour == hour and now.minute == minute:
                self._fire(job_id, params)
                self._fired_today.add(job_id)

    def _fire(self, job_id: str, params: dict) -> None:
        logger.info("Daily routine %s", job_id)
        self._bus.publish(
            "state.request",
            StateRequest(
                state=params.get("activity", "idle"),
                priority=Priority.NORMAL,
                source=CommandSource.ROUTINE,
                activity=params.get("activity"),
                life_mode=params.get("life_mode"),
                habit=params.get("habit"),
            ),
        )
