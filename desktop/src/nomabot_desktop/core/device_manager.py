"""Device registry - metadata and capabilities only (no transport)."""

from __future__ import annotations

import logging
from dataclasses import dataclass, field

logger = logging.getLogger("noma.desktop")


@dataclass
class DeviceRecord:
    device_id: str
    name: str
    transport_type: str
    transport_config: dict
    character_id: str = "nomabot"
    last_seen: int | None = None
    firmware_version: str | None = None
    protocol_version: int | None = None
    display_width: int | None = None
    display_height: int | None = None
    serial_number: str | None = None
    online: bool = False
    caps: list[str] = field(default_factory=list)


class DeviceManager:
    """Registry of devices - no transport access."""

    def __init__(self) -> None:
        self._devices: dict[str, DeviceRecord] = {}
        self._default_id: str | None = None

    def register(self, record: DeviceRecord, *, default: bool = False) -> None:
        self._devices[record.device_id] = record
        if default or self._default_id is None:
            self._default_id = record.device_id

    def get(self, device_id: str | None = None) -> DeviceRecord | None:
        did = device_id or self._default_id
        return self._devices.get(did) if did else None

    def update_from_hello(self, device_id: str, hello_data: dict) -> None:
        dev = self._devices.get(device_id)
        if not dev:
            return
        dev.online = True
        dev.firmware_version = hello_data.get("firmware") or hello_data.get("firmware_version")
        dev.protocol_version = hello_data.get("protocol")
        dev.serial_number = hello_data.get("serial")
        display = hello_data.get("display") or {}
        dev.display_width = display.get("width")
        dev.display_height = display.get("height")
        dev.caps = hello_data.get("caps") or []

    def set_offline(self, device_id: str) -> None:
        dev = self._devices.get(device_id)
        if dev:
            dev.online = False

    def default_id(self) -> str | None:
        return self._default_id
