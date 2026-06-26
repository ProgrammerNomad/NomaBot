"""Application context - wires core components."""

from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path

from nomabot_desktop.core.asset_registry import AssetRegistry
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_dispatcher import CommandDispatcher
from nomabot_desktop.core.device_manager import DeviceManager
from nomabot_desktop.core.priority_queue import PriorityQueue
from nomabot_desktop.core.runtime import NomaRuntime
from nomabot_desktop.core.state_manager import StateManager
from nomabot_desktop.services.config import ConfigService
from nomabot_desktop.storage.service import StorageService
from nomabot_desktop.transport import EmulatorState
from nomabot_desktop.transport.manager import TransportManager


@dataclass
class AppContext:
    bus: EventBus
    storage: StorageService
    config: ConfigService
    device_manager: DeviceManager
    transport_manager: TransportManager
    queue: PriorityQueue
    dispatcher: CommandDispatcher
    runtime: NomaRuntime
    state_manager: StateManager | None = None
    emu_state: EmulatorState | None = None
    log_dir: Path | None = None
    default_device_id: str | None = None


def create_context() -> AppContext:
    appdata = Path(os.environ.get("APPDATA", str(Path.home()))) / "NomaBot"
    storage = StorageService(appdata / "noma.db")
    config = ConfigService(storage)
    bus = EventBus()
    dm = DeviceManager()
    tm = TransportManager(bus)
    queue = PriorityQueue()
    dispatcher = CommandDispatcher(queue, tm)
    assets = AssetRegistry()
    runtime = NomaRuntime(queue, dispatcher, assets)

    return AppContext(
        bus=bus,
        storage=storage,
        config=config,
        device_manager=dm,
        transport_manager=tm,
        queue=queue,
        dispatcher=dispatcher,
        runtime=runtime,
    )
