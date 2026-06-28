"""Friendship service — long memory welcome messages on connect."""

from __future__ import annotations

import logging
import time
from dataclasses import dataclass

from nomabot.types import Priority
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_source import CommandSource
from nomabot_desktop.core.events import DeviceConnected, OverlayShow, StateRequest
from nomabot_desktop.storage.service import StorageService

logger = logging.getLogger("noma.friendship")


@dataclass(frozen=True)
class FriendshipMemory:
    first_seen_at: int
    days_together: int
    user_name: str | None = None


class FriendshipService:
    def __init__(self, bus: EventBus, storage: StorageService) -> None:
        self._bus = bus
        self._storage = storage
        bus.subscribe("device.connected", self._on_device_connected)

    def record_session(self) -> FriendshipMemory:
        now_ms = int(time.time() * 1000)
        first_seen = self._storage.get_long_memory("first_seen_at")
        if not first_seen:
            self._storage.set_long_memory("first_seen_at", str(now_ms))
            first_ms = now_ms
        else:
            first_ms = int(first_seen)
        days = max(1, (now_ms - first_ms) // (24 * 3600 * 1000) + 1)
        self._storage.set_long_memory("days_together", str(days))
        user_name = self._storage.get_long_memory("user_name")
        return FriendshipMemory(first_seen_at=first_ms, days_together=days, user_name=user_name)

    def welcome_message(self, memory: FriendshipMemory) -> str | None:
        if memory.days_together == 1:
            return "Hello."
        if memory.days_together >= 100 and memory.user_name:
            return f"Welcome back, {memory.user_name}!"
        if memory.days_together >= 100:
            return "Welcome back!"
        if memory.days_together % 30 == 0:
            return f"Day {memory.days_together} together."
        return None

    def _on_device_connected(self, payload: DeviceConnected) -> None:
        memory = self.record_session()
        text = self.welcome_message(memory)
        if not text:
            return
        logger.info("Friendship welcome (day %s)", memory.days_together)
        self._bus.publish(
            "state.request",
            StateRequest(
                state="idle",
                priority=Priority.NORMAL,
                source=CommandSource.USER,
                emotion="happy",
            ),
        )
        self._bus.publish(
            "overlay.show",
            OverlayShow(
                overlay_id="friendship_welcome",
                text=text,
                priority=Priority.NORMAL,
            ),
        )
