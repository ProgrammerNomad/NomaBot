"""Scheduler service - central job registry."""

from __future__ import annotations

import logging
import time

from PySide6.QtCore import QTimer

from nomabot.types import Priority
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.events import SchedulerFire
from nomabot_desktop.storage.service import StorageService

logger = logging.getLogger("noma.scheduler")


class SchedulerService:
    def __init__(self, bus: EventBus, storage: StorageService) -> None:
        self._bus = bus
        self._storage = storage
        self._last_fired: dict[str, float] = {}
        self._timer = QTimer()
        self._timer.timeout.connect(self._tick)
        self._timer.start(30_000)

    def start(self) -> None:
        self._storage.seed_default_jobs()
        logger.info("Scheduler started")

    def _tick(self) -> None:
        now = time.time()
        for job in self._storage.list_scheduler_jobs():
            if not job.enabled:
                continue
            last = self._last_fired.get(job.job_id, 0.0)
            if now - last < job.interval_seconds:
                continue
            self._last_fired[job.job_id] = now
            try:
                priority = Priority[job.priority]
            except KeyError:
                priority = Priority.LOW
            payload = SchedulerFire(
                job_id=job.job_id,
                action=job.action,
                parameters=job.parameters,
                priority=priority,
            )
            logger.info("Scheduler firing %s (%s)", job.job_id, job.action)
            self._bus.publish("scheduler.fire", payload)

    def stop(self) -> None:
        self._timer.stop()
