"""Application lifecycle."""

from __future__ import annotations

import asyncio
import json
import logging
import threading
from collections.abc import Coroutine
from pathlib import Path
from typing import Any

from PySide6.QtWidgets import QApplication, QMainWindow, QPushButton, QVBoxLayout, QWidget

from nomabot.testing import MockDevice
from nomabot.types import MessageSpec, Priority, RenderRequest
from nomabot_desktop.core.device_manager import Device, DeviceManager
from nomabot_desktop.core.logging_config import setup_logging
from nomabot_desktop.core.runtime import NomaRuntime
from nomabot_desktop.transport import EmulatorState, EmulatorTransport, TransportAdapter
from nomabot_desktop.ui.emulator import EmulatorWindow

logger = logging.getLogger("noma.desktop")

_bg_loop: asyncio.AbstractEventLoop | None = None


def _bg_async_loop() -> asyncio.AbstractEventLoop:
    """Dedicated asyncio loop for device I/O (runs off the Qt main thread)."""
    global _bg_loop
    if _bg_loop is None:
        loop = asyncio.new_event_loop()

        def _run() -> None:
            asyncio.set_event_loop(loop)
            loop.run_forever()

        threading.Thread(target=_run, daemon=True, name="noma-async").start()
        _bg_loop = loop
    return _bg_loop


def _schedule[T](coro: Coroutine[Any, Any, T]) -> asyncio.Future[T]:
    return asyncio.run_coroutine_threadsafe(coro, _bg_async_loop())


def _load_profile() -> dict:
    root = Path(__file__).resolve().parents[4]
    return json.loads((root / "profiles" / "lilygo_tdisplay_s3.json").read_text(encoding="utf-8"))


async def _bootstrap(
    *, emulator: bool, port: str | None
) -> tuple[NomaRuntime, DeviceManager, dict, EmulatorState]:
    profile = _load_profile()
    dm = DeviceManager()

    emu_state = EmulatorState(
        width=profile["display"]["width"],
        height=profile["display"]["height"],
    )

    if port:
        from nomabot.transport.serial import SerialTransport

        inner = SerialTransport(port, profile["default_baud"])
        transport = TransportAdapter(inner)
        dm.register(Device("device-1", "USB Device", transport), default=True)
    elif emulator:
        inner = EmulatorTransport(emu_state)
        dm.register(Device("emulator", "Emulator", inner), default=True)
    else:
        inner = MockDevice()
        dm.register(Device("mock", "Mock Device", inner), default=True)

    runtime = NomaRuntime(dm)
    await dm.connect_all()
    return runtime, dm, profile, emu_state


def run_app(*, emulator: bool = False, port: str | None = None) -> None:
    setup_logging()
    logger.info("NomaBot desktop 0.1.0 starting")

    runtime, dm, _, emu_state = _schedule(_bootstrap(emulator=emulator, port=port)).result()

    app = QApplication([])
    window = QMainWindow()
    window.setWindowTitle("NomaBot Desktop 0.1.0")
    central = QWidget()
    layout = QVBoxLayout(central)

    btn_idle = QPushButton("Play idle")
    btn_coding = QPushButton("Play coding")
    btn_say = QPushButton('Say "Hello"')

    async def play_idle():
        await runtime.submit(RenderRequest(animation="idle", priority=Priority.NORMAL))

    async def play_coding():
        await runtime.submit(RenderRequest(animation="coding", priority=Priority.NORMAL))

    async def say_hello():
        await runtime.submit(
            RenderRequest(
                message=MessageSpec(text="Hello"),
                priority=Priority.NORMAL,
            )
        )

    btn_idle.clicked.connect(lambda: _schedule(play_idle()))
    btn_coding.clicked.connect(lambda: _schedule(play_coding()))
    btn_say.clicked.connect(lambda: _schedule(say_hello()))

    layout.addWidget(btn_idle)
    layout.addWidget(btn_coding)
    layout.addWidget(btn_say)
    window.setCentralWidget(central)
    window.resize(300, 200)
    window.show()

    emu_win = None
    if emulator:
        emu_win = EmulatorWindow(emu_state)
        emu_win.show()

    # Demo: auto-play idle on start
    _schedule(runtime.submit(RenderRequest(animation="idle", priority=Priority.NORMAL)))

    app.exec()
    _schedule(dm.disconnect_all()).result()
