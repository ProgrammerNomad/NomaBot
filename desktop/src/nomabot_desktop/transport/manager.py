"""Transport factory and lifecycle - sole owner of transport instances."""

from __future__ import annotations

import logging
from typing import Any

from nomabot.client import NomaClient
from nomabot.protocol.envelope import Envelope
from nomabot.testing import MockDevice
from nomabot.transport.serial import SerialTransport
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.events import DeviceConnected, DeviceDisconnected
from nomabot_desktop.transport import EmulatorTransport, TransportAdapter

logger = logging.getLogger("noma.transport")


class TransportManager:
    def __init__(self, bus: EventBus) -> None:
        self._bus = bus
        self._adapters: dict[str, TransportAdapter] = {}
        self._configs: dict[str, dict[str, Any]] = {}
        self._hello_data: dict[str, dict[str, Any]] = {}

    def register(
        self,
        device_id: str,
        transport_type: str,
        config: dict[str, Any],
        *,
        inner: Any | None = None,
    ) -> None:
        self._configs[device_id] = {"type": transport_type, **config}
        if inner is not None:
            adapter = TransportAdapter(inner)
        elif transport_type == "serial":
            adapter = TransportAdapter(SerialTransport(config["port"], config.get("baud", 115200)))
        elif transport_type == "emulator":
            adapter = TransportAdapter(EmulatorTransport(config["state"]))
        elif transport_type == "mock":
            adapter = TransportAdapter(MockDevice())
        else:
            raise ValueError(f"Unknown transport type: {transport_type}")

        self._adapters[device_id] = adapter

    async def connect(self, device_id: str) -> dict[str, Any]:
        adapter = self._adapters.get(device_id)
        if not adapter:
            raise KeyError(f"No transport for {device_id}")

        inner = adapter._inner  # noqa: SLF001
        client = NomaClient(inner)
        hello_env = await client.connect()
        data = hello_env.data or {}
        self._hello_data[device_id] = data
        self._bus.publish(
            "device.connected",
            DeviceConnected(
                device_id=device_id,
                firmware_version=data.get("firmware") or data.get("firmware_version"),
                display_width=(data.get("display") or {}).get("width"),
                display_height=(data.get("display") or {}).get("height"),
            ),
        )
        logger.info("Connected device %s", device_id)
        return data

    async def disconnect(self, device_id: str) -> None:
        adapter = self._adapters.get(device_id)
        if adapter:
            await adapter.disconnect()
            self._bus.publish("device.disconnected", DeviceDisconnected(device_id=device_id))
            logger.info("Disconnected device %s", device_id)

    async def disconnect_all(self) -> None:
        for device_id in list(self._adapters):
            await self.disconnect(device_id)

    async def send(self, device_id: str, data: bytes) -> None:
        adapter = self._adapters.get(device_id)
        if not adapter:
            logger.warning("No transport for send to %s", device_id)
            return
        logger.debug("Send %s", data.decode("utf-8", errors="replace").strip())
        await adapter.send(data)

    async def send_envelope(self, device_id: str, envelope: Envelope) -> None:
        await self.send(device_id, envelope.to_json_line().encode("utf-8"))

    def get_hello_data(self, device_id: str) -> dict[str, Any]:
        return self._hello_data.get(device_id, {})
