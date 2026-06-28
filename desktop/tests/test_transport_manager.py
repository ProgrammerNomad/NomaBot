"""Transport manager tests."""

import pytest

from nomabot.testing import MockDevice
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.transport.manager import TransportManager


@pytest.mark.asyncio
async def test_connect_hello_rich_fields() -> None:
    bus = EventBus()
    tm = TransportManager(bus)
    mock = MockDevice()
    tm.register("d1", "mock", {}, inner=mock)
    data = await tm.connect("d1")
    assert data.get("protocol") == 1
    assert data.get("firmware") == "0.3.1"
    assert data["display"]["width"] == 170
    await tm.disconnect_all()
