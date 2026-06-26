"""Device manager v0 — single/multi device routing."""

from __future__ import annotations

import logging
from typing import Protocol

from nomabot.protocol.envelope import Envelope

logger = logging.getLogger("noma.transport")


class DeviceTransport(Protocol):
    async def connect(self) -> None: ...
    async def disconnect(self) -> None: ...
    async def send(self, data: bytes) -> None: ...


class Device:
    def __init__(self, device_id: str, name: str, transport: DeviceTransport) -> None:
        self.device_id = device_id
        self.name = name
        self.transport = transport
        self.online = False


class DeviceManager:
    def __init__(self) -> None:
        self._devices: dict[str, Device] = {}
        self._default_id: str | None = None

    def register(self, device: Device, *, default: bool = False) -> None:
        self._devices[device.device_id] = device
        if default or self._default_id is None:
            self._default_id = device.device_id

    def get(self, device_id: str | None = None) -> Device | None:
        did = device_id or self._default_id
        return self._devices.get(did) if did else None

    async def connect_all(self) -> None:
        for dev in self._devices.values():
            await dev.transport.connect()
            dev.online = True
            logger.info("Connected device %s", dev.device_id)

    async def disconnect_all(self) -> None:
        for dev in self._devices.values():
            await dev.transport.disconnect()
            dev.online = False

    async def send_envelope(self, envelope: Envelope, device_id: str | None = None) -> None:
        dev = self.get(device_id)
        if not dev:
            logger.warning("No device for send")
            return
        line = envelope.to_json_line().encode("utf-8")
        logger.debug("Send %s", line.decode().strip())
        await dev.transport.send(line)
