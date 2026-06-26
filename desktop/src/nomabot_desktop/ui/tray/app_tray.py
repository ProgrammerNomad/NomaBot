"""System tray icon and menu."""

from __future__ import annotations

from collections.abc import Callable

from PySide6.QtGui import QAction, QIcon
from PySide6.QtWidgets import (
    QApplication,
    QLabel,
    QMenu,
    QSystemTrayIcon,
    QWidgetAction,
)

from nomabot_desktop.core.app_context import AppContext
from nomabot_desktop.core.device_manager import DeviceRecord


class AppTray:
    def __init__(
        self,
        ctx: AppContext,
        *,
        on_settings: Callable[[], None],
        on_connect: Callable[[], None],
        on_disconnect: Callable[[], None],
        on_quit: Callable[[], None],
        on_toggle_mute: Callable[[bool], None],
        on_show_dev: Callable[[], None] | None = None,
    ) -> None:
        self._ctx = ctx
        self._callbacks = {
            "settings": on_settings,
            "connect": on_connect,
            "disconnect": on_disconnect,
            "quit": on_quit,
            "mute": on_toggle_mute,
            "show_dev": on_show_dev,
        }
        self._status_text = "Status: offline"
        self._muted = False

        app = QApplication.instance()
        self._tray = QSystemTrayIcon(app)
        icon = QIcon.fromTheme("computer")
        if icon.isNull():
            icon = app.style().standardIcon(app.style().StandardPixmap.SP_ComputerIcon)
        self._tray.setIcon(icon)
        self._tray.setToolTip("NomaBot — starting…")

        self._menu = QMenu()
        self._menu.setMinimumWidth(200)
        self._menu.aboutToShow.connect(self._rebuild_menu)
        self._tray.setContextMenu(self._menu)
        self._tray.show()

        ctx.bus.subscribe("device.connected", self._on_connected)
        ctx.bus.subscribe("device.disconnected", self._on_disconnected)

    def _rebuild_menu(self) -> None:
        """Rebuild on each open — avoids Windows tray menu clipping bugs."""
        self._menu.clear()

        status_label = QLabel(self._status_text)
        status_label.setStyleSheet("color: gray; padding: 4px 12px;")
        status_action = QWidgetAction(self._menu)
        status_action.setDefaultWidget(status_label)
        self._menu.addAction(status_action)

        self._menu.addSeparator()

        connect_act = QAction("Connect", self._menu)
        connect_act.triggered.connect(self._callbacks["connect"])
        self._menu.addAction(connect_act)

        disconnect_act = QAction("Disconnect", self._menu)
        disconnect_act.triggered.connect(self._callbacks["disconnect"])
        self._menu.addAction(disconnect_act)

        mute_act = QAction("Mute", self._menu)
        mute_act.setCheckable(True)
        mute_act.setChecked(self._muted)
        mute_act.toggled.connect(self._callbacks["mute"])
        self._menu.addAction(mute_act)

        if self._callbacks.get("show_dev"):
            dev_act = QAction("Show dev controls", self._menu)
            dev_act.triggered.connect(self._callbacks["show_dev"])
            self._menu.addAction(dev_act)

        self._menu.addSeparator()

        settings_act = QAction("Settings…", self._menu)
        settings_act.triggered.connect(self._callbacks["settings"])
        self._menu.addAction(settings_act)

        quit_act = QAction("Quit", self._menu)
        quit_act.triggered.connect(self._callbacks["quit"])
        self._menu.addAction(quit_act)

    def _on_connected(self, _payload) -> None:
        dev = self._ctx.device_manager.get()
        self._update_status(dev)

    def _on_disconnected(self, _payload) -> None:
        self._status_text = "Status: offline"
        self._tray.setToolTip("NomaBot — offline")

    def _update_status(self, dev: DeviceRecord | None) -> None:
        if not dev:
            return
        fw = dev.firmware_version or "?"
        res = f"{dev.display_width or '?'}×{dev.display_height or '?'}"
        char = ""
        cs = getattr(self._ctx, "character_service", None)
        if cs and cs.active_pack_id:
            short_uuid = (cs.active_uuid or "")[:8]
            char = f", {cs.active_pack_id}"
            if short_uuid:
                char += f" ({short_uuid}…)"
        self._status_text = f"Status: online ({fw}, {res}{char})"
        self._tray.setToolTip(f"NomaBot — {dev.name} — {fw} — {res}{char}")

    def set_muted(self, muted: bool) -> None:
        self._muted = muted
