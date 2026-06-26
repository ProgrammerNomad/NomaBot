"""Runtime priority merge tests."""

import pytest

from nomabot.testing import MockDevice
from nomabot.types import Priority, RenderRequest
from nomabot_desktop.core.device_manager import Device, DeviceManager
from nomabot_desktop.core.runtime import NomaRuntime


@pytest.mark.asyncio
async def test_priority_merge_animation():
    mock = MockDevice()
    dm = DeviceManager()
    dm.register(Device("d1", "Mock", mock), default=True)
    await dm.connect_all()
    runtime = NomaRuntime(dm)

    await runtime.submit(RenderRequest(animation="idle", priority=Priority.LOW))
    await runtime.submit(RenderRequest(animation="coding", priority=Priority.HIGH))

    assert mock.last_animation == "coding"


@pytest.mark.asyncio
async def test_low_priority_does_not_override_high():
    mock = MockDevice()
    dm = DeviceManager()
    dm.register(Device("d1", "Mock", mock), default=True)
    await dm.connect_all()
    runtime = NomaRuntime(dm)

    await runtime.submit(RenderRequest(animation="coding", priority=Priority.HIGH))
    await runtime.submit(RenderRequest(animation="idle", priority=Priority.LOW))

    assert mock.last_animation == "coding"
