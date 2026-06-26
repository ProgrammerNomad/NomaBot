"""Integration: runtime + mock device."""

import pytest

from nomabot.testing import MockDevice
from nomabot.types import Priority, RenderRequest
from nomabot_desktop.core.device_manager import Device, DeviceManager
from nomabot_desktop.core.runtime import NomaRuntime


@pytest.mark.asyncio
async def test_runtime_to_mock_play_animation():
    mock = MockDevice()
    dm = DeviceManager()
    dm.register(Device("d1", "Mock", mock), default=True)
    await dm.connect_all()
    runtime = NomaRuntime(dm)

    await runtime.submit(RenderRequest(animation="coding", priority=Priority.NORMAL))

    assert mock.last_animation == "coding"
    assert len(mock.sent_envelopes) >= 1
    assert any(e.cmd == "play_animation" for e in mock.sent_envelopes)
