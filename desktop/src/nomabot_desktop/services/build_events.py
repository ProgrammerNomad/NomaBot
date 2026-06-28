"""Build event service — emotion + message on build ok/fail."""

from __future__ import annotations

import logging
from dataclasses import dataclass

from nomabot.types import Priority
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.events import StateRequest

logger = logging.getLogger("noma.build_events")


@dataclass(frozen=True)
class BuildResult:
    success: bool
    message: str = ""


class BuildEventService:
    def __init__(self, bus: EventBus) -> None:
        self._bus = bus
        bus.subscribe("build.result", self._on_build_result)

    def _on_build_result(self, payload: BuildResult) -> None:
        if payload.success:
            logger.info("Build success")
            self._bus.publish(
                "state.request",
                StateRequest(
                    state="coding",
                    priority=Priority.HIGH,
                    source="build",
                    emotion="excited",
                    message_text=payload.message or "Build OK",
                ),
            )
        else:
            logger.info("Build failed")
            self._bus.publish(
                "state.request",
                StateRequest(
                    state="coding",
                    priority=Priority.HIGH,
                    source="build",
                    emotion="frustrated",
                    message_text=payload.message or "Build failed",
                ),
            )
