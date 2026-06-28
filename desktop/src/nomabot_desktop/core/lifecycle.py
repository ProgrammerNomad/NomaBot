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

from serial.serialutil import SerialException

from nomabot.types import Priority
from nomabot_desktop.core.command_source import CommandSource
from nomabot_desktop.core.app_context import AppContext, create_context
from nomabot_desktop.core.device_manager import DeviceRecord
from nomabot_desktop.core.command_router import CommandRouter
from nomabot_desktop.core.events import OverlayShow, StateRequest
from nomabot_desktop.core.logging_config import setup_logging
from nomabot_desktop.core.overlay_service import OverlayService
from nomabot_desktop.core.state_manager import BotState, StateManager
from nomabot_desktop.services.activity import ActivityService
from nomabot_desktop.services.build_events import BuildEventService
from nomabot_desktop.services.character import CharacterService
from nomabot_desktop.services.daily_routine import DailyRoutineService
from nomabot_desktop.services.firmware_compat import log_firmware_issues
from nomabot_desktop.services.friendship import FriendshipService
from nomabot_desktop.services.life_mode import LifeModeService
from nomabot_desktop.services.scheduler import SchedulerService
from nomabot_desktop.services.season import SeasonService
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
    fw_ok = log_firmware_issues(hello)
    if ctx.character_service and fw_ok:
        await ctx.character_service.activate(device_id, "nomabot")
    elif ctx.character_service and not fw_ok:
        logger.error(
            "Skipping load_character until firmware is 0.3.0+ with load_character cap. "
            "Run: cd firmware && pio run -e lilygo_tdisplay_s3 -t upload && "
            "pio run -e lilygo_tdisplay_s3 -t uploadfs — then RESET the board."
        )


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
    ctx.emu_state.activity = state.activity
    ctx.emu_state.emotion = state.emotion
    ctx.emu_state.life_mode = state.life_mode
    ctx.emu_state.animation = state.animation
    ctx.emu_state.render_mode = "text"


def _sync_emulator_overlay(ctx: AppContext, overlay: OverlayShow) -> None:
    if ctx.emu_state is None:
        return
    ctx.emu_state.message = overlay.text
    ctx.emu_state.overlay_id = overlay.overlay_id


def _toggle_mute(ctx: AppContext, tray: AppTray | None, muted: bool) -> None:
    ctx.config.muted = muted
    if ctx.state_manager:
        ctx.state_manager.set_muted(muted)
    if tray:
        tray.set_muted(muted)


def _build_dev_window(ctx: AppContext) -> QMainWindow:
    window = QMainWindow()
    window.setWindowTitle("NomaBot Dev Controls")
    central = QWidget()
    layout = QVBoxLayout(central)

    def req(state: str, **kwargs):
        if ctx.router is None:
            return
        ctx.router.context(
            StateRequest(
                state=state,
                priority=Priority.NORMAL,
                source=CommandSource.DEV_PANEL,
                **kwargs,
            )
        )

    def say_hello() -> None:
        if ctx.router is None:
            return
        ctx.router.overlay(overlay_id="dev_hello", text="Hello")

    btn_idle = QPushButton("Activity: idle")
    btn_idle.clicked.connect(lambda: req("idle"))
    btn_coding = QPushButton("Activity: coding")
    btn_coding.clicked.connect(lambda: req("coding"))
    btn_sleep = QPushButton("Activity: sleep")
    btn_sleep.clicked.connect(lambda: req("sleep"))
    btn_happy = QPushButton("Emotion: happy")
    btn_happy.clicked.connect(lambda: req("idle", emotion="happy"))
    btn_frustrated = QPushButton("Emotion: frustrated")
    btn_frustrated.clicked.connect(lambda: req("coding", emotion="frustrated"))
    btn_say = QPushButton('Say "Hello"')
    btn_say.clicked.connect(say_hello)
    btn_home = QPushButton("Life mode: home")
    btn_home.clicked.connect(lambda: req("idle", life_mode="home"))
    btn_morning = QPushButton("Habit: morning")
    btn_morning.clicked.connect(lambda: req("idle", habit="morning"))
    btn_build_ok = QPushButton("Build OK")
    btn_build_fail = QPushButton("Build fail")

    from nomabot_desktop.services.build_events import BuildResult

    btn_build_ok.clicked.connect(
        lambda: ctx.bus.publish("build.result", BuildResult(success=True, message="Build OK"))
    )
    btn_build_fail.clicked.connect(
        lambda: ctx.bus.publish("build.result", BuildResult(success=False, message="Build failed"))
    )

    layout.addWidget(btn_idle)
    layout.addWidget(btn_coding)
    layout.addWidget(btn_sleep)
    layout.addWidget(btn_happy)
    layout.addWidget(btn_frustrated)
    layout.addWidget(btn_say)
    layout.addWidget(btn_home)
    layout.addWidget(btn_morning)
    layout.addWidget(btn_build_ok)
    layout.addWidget(btn_build_fail)
    window.setCentralWidget(central)
    window.resize(320, 380)
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
    logger.info("NomaBot desktop 0.4.1 starting")

    ctx = create_context()
    ctx.log_dir = log_dir
    ctx.character_service = CharacterService(ctx.runtime.assets, ctx.transport_manager)

    overlay = OverlayService(ctx.bus, ctx.queue, ctx.dispatcher, _schedule)
    ctx.overlay = overlay

    state_manager = StateManager(ctx.bus, _schedule)
    state_manager.bind_runtime(ctx.runtime.submit, ctx.runtime.submit_renderer)
    state_manager.set_muted(ctx.config.muted)
    ctx.state_manager = state_manager

    ctx.router = CommandRouter(state_manager, overlay, ctx.runtime, _schedule)

    ctx.bus.subscribe("state.changed", lambda s: _sync_emulator_state(ctx, s))
    ctx.bus.subscribe("overlay.changed", lambda o: _sync_emulator_overlay(ctx, o))

    try:
        device_id, emu_state = _schedule(
            _bootstrap_hardware(ctx, emulator=emulator, port=port)
        ).result()
    except SerialException as exc:
        if port:
            logger.error(
                "Cannot open %s (%s). Close PlatformIO serial monitor, "
                "any other NomaBot instance, and Arduino IDE serial — then retry.",
                port,
                exc,
            )
        else:
            logger.error("Serial connect failed: %s", exc)
        raise SystemExit(1) from exc

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

    life_mode = LifeModeService(ctx.bus)
    life_mode.start()
    daily_routine = DailyRoutineService(ctx.bus)
    daily_routine.start()
    season = SeasonService(ctx.bus)
    season.start()
    BuildEventService(ctx.bus)
    FriendshipService(ctx.bus, ctx.storage)

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
        StateRequest(
            state="idle",
            priority=Priority.NORMAL,
            source=CommandSource.SYSTEM,
            animation="idle",
        ),
    )

    app.exec()

    activity.stop()
    scheduler.stop()
    life_mode.stop()
    daily_routine.stop()
    season.stop()
    _schedule(ctx.transport_manager.disconnect_all()).result()
    ctx.storage.close()
