"""Serial transport implementation."""

from __future__ import annotations

import asyncio
from collections.abc import Callable
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    pass


class SerialTransport:
    """USB serial transport using pyserial."""

    def __init__(self, port: str, baud: int = 115200) -> None:
        self._port = port
        self._baud = baud
        self._serial = None
        self._callback: Callable[[bytes], None] | None = None
        self._reader_task: asyncio.Task | None = None
        self._running = False

    async def connect(self) -> None:
        import serial

        self._serial = serial.Serial(self._port, self._baud, timeout=0.1)
        self._running = True
        self._reader_task = asyncio.create_task(self._read_loop())

    async def disconnect(self) -> None:
        self._running = False
        if self._reader_task:
            self._reader_task.cancel()
            try:
                await self._reader_task
            except asyncio.CancelledError:
                pass
        if self._serial and self._serial.is_open:
            self._serial.close()
        self._serial = None

    def on_receive(self, callback: Callable[[bytes], None]) -> None:
        self._callback = callback

    async def send(self, data: bytes) -> None:
        if not self._serial or not self._serial.is_open:
            raise RuntimeError("serial not connected")
        await asyncio.to_thread(self._serial.write, data)
        await asyncio.to_thread(self._serial.flush)

    async def _read_loop(self) -> None:
        while self._running and self._serial and self._serial.is_open:
            data = await asyncio.to_thread(self._serial.read, 256)
            if data and self._callback:
                self._callback(data)
            await asyncio.sleep(0.01)
