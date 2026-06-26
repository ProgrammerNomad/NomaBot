"""Application lifecycle."""

from __future__ import annotations

import asyncio
import json
import logging
import threading
import time
from collections.abc import Coroutine
from pathlib import Path
from typing import Any

from PySide6.QtWidgets import QApplication, QMainWindow, QPushButton, QVBoxLayout, QWidget

from nomabot.types import Priority
from nomabot_desktop.core.app_context import AppContext, create_context
from nomabot_desktop.core.device_manager import DeviceRecord
from nomabot_desktop.core.events import StateRequest
from nomabot_desktop.core.logging_config import setup_logging
from nomabot_desktop.core.state_manager import BotState, StateManager
from nomabot_desktop.services.activity import ActivityService
from nomabot_desktop.services.character import CharacterService
from nomabot_desktop.services.scheduler import SchedulerService
from nomabot_desktop.storage.service import DeviceRow
from nomabot_desktop.transport import EmulatorState
from nomabot_desktop.ui.emulator import EmulatorWindow
from nomabot_desktop.ui.settings_dialog import SettingsDialog
from nomabot_desktop.ui.tray.app_tray import AppTray

logger = logging.getLogger("noma.desktop")

_bg_loop: asyncio.AbstractEventLoop | None = None


def _bg_async_loop() -> asyncio.AbstractEventLoop:
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


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[4]


def _load_profile() -> dict:
    path = _repo_root() / "profiles" / "lilygo_tdisplay_s3.json"
    return json.loads(path.read_text(encoding="utf-8"))


def _persist_device(ctx: AppContext, device_id: str, hello_data: dict) -> None:
    dev = ctx.device_manager.get(device_id)
    if not dev:
        return
    ctx.storage.upsert_device(
        DeviceRow(
            device_id=dev.device_id,
            name=dev.name,
            transport_type=dev.transport_type,
            transport_config=dev.transport_config,
            character_id=dev.character_id,
            last_seen=int(time.time() * 1000),
            firmware_version=dev.firmware_version,
            protocol_version=dev.protocol_version,
            display_width=dev.display_width,
            display_height=dev.display_height,
            serial_number=dev.serial_number,
            online=dev.online,
        )
    )


async def _connect_device(ctx: AppContext, device_id: str) -> None:
    hello = await ctx.transport_manager.connect(device_id)
    ctx.device_manager.update_from_hello(device_id, hello)
    _persist_device(ctx, device_id, hello)
    if ctx.character_service:
        await ctx.character_service.activate(device_id, "nomabot")


async def _bootstrap_hardware(
    ctx: AppContext,
    *,
    emulator: bool,
    port: str | None,
) -> tuple[str, EmulatorState | None]:
    profile = _load_profile()
    emu_state: EmulatorState | None = None

    if port:
        device_id = "device-1"
        transport_config = {"port": port, "baud": profile["default_baud"]}
        ctx.device_manager.register(
            DeviceRecord(
                device_id=device_id,
                name="USB Device",
                transport_type="serial",
                transport_config=transport_config,
            ),
            default=True,
        )
        ctx.transport_manager.register(device_id, "serial", transport_config)
        ctx.config.last_port = port
    elif emulator:
        device_id = "emulator"
        emu_state = EmulatorState(
            width=profile["display"]["width"],
            height=profile["display"]["height"],
        )
        ctx.emu_state = emu_state
        ctx.device_manager.register(
            DeviceRecord(
                device_id=device_id,
                name="Emulator",
                transport_type="emulator",
                transport_config={},
            ),
            default=True,
        )
        ctx.transport_manager.register(device_id, "emulator", {"state": emu_state})
    else:
        use_port = ctx.config.last_port
        if use_port:
            device_id = "device-1"
            transport_config = {"port": use_port, "baud": profile["default_baud"]}
            ctx.device_manager.register(
                DeviceRecord(
                    device_id=device_id,
                    name="USB Device",
                    transport_type="serial",
                    transport_config=transport_config,
                ),
                default=True,
            )
            ctx.transport_manager.register(device_id, "serial", transport_config)
        else:
            device_id = "emulator"
            emu_state = EmulatorState(
                width=profile["display"]["width"],
                height=profile["display"]["height"],
            )
            ctx.emu_state = emu_state
            ctx.device_manager.register(
                DeviceRecord(
                    device_id=device_id,
                    name="Emulator",
                    transport_type="emulator",
                    transport_config={},
                ),
                default=True,
            )
            ctx.transport_manager.register(device_id, "emulator", {"state": emu_state})

    ctx.default_device_id = device_id
    ctx.dispatcher.set_default_device(device_id)
    await _connect_device(ctx, device_id)
    return device_id, emu_state


def _sync_emulator_state(ctx: AppContext, state: BotState) -> None:
    if ctx.emu_state is None:
        return
    ctx.emu_state.animation = state.animation
    ctx.emu_state.message = state.message_text


def _build_dev_window(ctx: AppContext) -> QMainWindow:
    window = QMainWindow()
    window.setWindowTitle("NomaBot Dev Controls")
    central = QWidget()
    layout = QVBoxLayout(central)

    def req(state: str, **kwargs):
        ctx.bus.publish(
            "state.request",
            StateRequest(state=state, priority=Priority.NORMAL, source="dev", **kwargs),
        )

    btn_idle = QPushButton("Play idle")
    btn_idle.clicked.connect(lambda: req("idle", animation="idle"))
    btn_coding = QPushButton("Play coding")
    btn_coding.clicked.connect(lambda: req("coding", animation="coding"))
    btn_say = QPushButton('Say "Hello"')
    btn_say.clicked.connect(lambda: req("message_active", message_text="Hello"))

    layout.addWidget(btn_idle)
    layout.addWidget(btn_coding)
    layout.addWidget(btn_say)
    window.setCentralWidget(central)
    window.resize(300, 200)
    return window


def run_app(
    *,
    emulator: bool = False,
    port: str | None = None,
    dev: bool = False,
    no_activity: bool = False,
    no_tray: bool = False,
) -> None:
    log_dir = setup_logging()
    logger.info("NomaBot desktop 0.3.0 starting")

    ctx = create_context()
    ctx.log_dir = log_dir
    ctx.character_service = CharacterService(ctx.runtime.assets, ctx.transport_manager)

    state_manager = StateManager(ctx.bus, _schedule)
    state_manager.bind_runtime(ctx.runtime.submit)
    state_manager.set_muted(ctx.config.muted)
    ctx.state_manager = state_manager

    ctx.bus.subscribe("state.changed", lambda s: _sync_emulator_state(ctx, s))

    device_id, emu_state = _schedule(
        _bootstrap_hardware(ctx, emulator=emulator, port=port)
    ).result()

    app = QApplication([])
    app.setQuitOnLastWindowClosed(False)

    emu_win: EmulatorWindow | None = None
    if emu_state is not None:
        emu_win = EmulatorWindow(emu_state, ctx.runtime.assets)
        emu_win.show()

    dev_window: QMainWindow | None = None
    if dev or no_tray:
        dev_window = _build_dev_window(ctx)
        dev_window.show()

    def show_dev_window() -> None:
        nonlocal dev_window
        if dev_window is None:
            dev_window = _build_dev_window(ctx)
        dev_window.show()
        dev_window.raise_()

    scheduler = SchedulerService(ctx.bus, ctx.storage)
    scheduler.start()

    activity = ActivityService(ctx.bus, ctx.config)
    if not no_activity:
        activity.start()

    settings_dialog: SettingsDialog | None = None

    def open_settings() -> None:
        nonlocal settings_dialog
        if settings_dialog is None:
            settings_dialog = SettingsDialog(ctx, log_dir)
        settings_dialog.show()
        settings_dialog.raise_()

    tray: AppTray | None = None

    def on_mute_toggled(muted: bool) -> None:
        _toggle_mute(ctx, tray, muted)

    if not no_tray:
        tray = AppTray(
            ctx,
            on_settings=open_settings,
            on_connect=lambda: _schedule(_connect_device(ctx, device_id)),
            on_disconnect=lambda: _schedule(ctx.transport_manager.disconnect(device_id)),
            on_quit=app.quit,
            on_toggle_mute=on_mute_toggled,
            on_show_dev=show_dev_window,
        )
        tray.set_muted(ctx.config.muted)
        dev = ctx.device_manager.get(device_id)
        if dev and dev.online:
            tray._update_status(dev)  # noqa: SLF001
        if ctx.character_service and ctx.character_service.active_pack_id:
            dev2 = ctx.device_manager.get(device_id)
            tray._update_status(dev2)  # noqa: SLF001

    ctx.bus.publish(
        "state.request",
        StateRequest(state="idle", priority=Priority.NORMAL, source="startup", animation="idle"),
    )

    app.exec()

    activity.stop()
    scheduler.stop()
    _schedule(ctx.transport_manager.disconnect_all()).result()
    ctx.storage.close()


    def _toggle_mute(ctx: AppContext, tray: AppTray | None, muted: bool) -> None:
        ctx.config.muted = muted
        ctx.state_manager.set_muted(muted)  # type: ignore[union-attr]
        if tray:
            tray.set_muted(muted)
