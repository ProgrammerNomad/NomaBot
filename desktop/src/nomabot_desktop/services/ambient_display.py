"""Optional ambient display modes for eyes character."""

from __future__ import annotations

import logging

from PySide6.QtCore import QTimer

logger = logging.getLogger("noma.ambient")


class AmbientDisplayService:
    """Placeholder rotator - weather glance timing handled by WeatherService interval."""

    def __init__(self) -> None:
        self._timer = QTimer()

    def start(self) -> None:
        logger.info("AmbientDisplayService started")
        self._timer.start(300_000)

    def stop(self) -> None:
        self._timer.stop()
