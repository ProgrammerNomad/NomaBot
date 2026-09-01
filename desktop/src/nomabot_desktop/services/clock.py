"""Push local clock to device ambient bar."""

from __future__ import annotations

import logging
from datetime import datetime

from PySide6.QtCore import QTimer

from nomabot.protocol.commands import SetClockParams, build_command
from nomabot_desktop.core.command_dispatcher import CommandDispatcher

logger = logging.getLogger("noma.clock")


class ClockService:
    def __init__(self, dispatcher: CommandDispatcher, device_id: str) -> None:
        self._dispatcher = dispatcher
        self._device_id = device_id
        self._timer = QTimer()
        self._timer.timeout.connect(self._tick)

    def start(self) -> None:
        self._tick()
        self._timer.start(60_000)
        logger.info("ClockService started")

    def stop(self) -> None:
        self._timer.stop()

    def _tick(self) -> None:
        now = datetime.now()
        time_text = now.strftime("%H:%M")
        cmd = build_command("set_clock", SetClockParams(time=time_text, date=now.strftime("%a %d %b")))
        self._dispatcher.enqueue(cmd, device_id=self._device_id)
