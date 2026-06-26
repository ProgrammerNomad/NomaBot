"""Settings dialog."""

from __future__ import annotations

from pathlib import Path

from PySide6.QtWidgets import (
    QCheckBox,
    QDialog,
    QDialogButtonBox,
    QFormLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QTabWidget,
    QVBoxLayout,
    QWidget,
)

from nomabot_desktop.core.app_context import AppContext
from nomabot_desktop.ui.widgets.log_viewer import LogViewerWidget


class SettingsDialog(QDialog):
    def __init__(self, ctx: AppContext, log_dir: Path, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._ctx = ctx
        self.setWindowTitle("NomaBot Settings")
        self.resize(520, 420)

        tabs = QTabWidget()
        tabs.addTab(self._build_general(), "General")
        tabs.addTab(LogViewerWidget(log_dir), "Logs")
        tabs.addTab(QLabel("Devices - coming soon"), "Devices")

        buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Close)
        buttons.rejected.connect(self.reject)

        layout = QVBoxLayout(self)
        layout.addWidget(tabs)
        layout.addWidget(buttons)

    def _build_general(self) -> QWidget:
        w = QWidget()
        form = QFormLayout(w)

        self._activity_cb = QCheckBox()
        self._activity_cb.setChecked(self._ctx.config.activity_enabled)
        self._activity_cb.toggled.connect(
            lambda v: setattr(self._ctx.config, "activity_enabled", v)
        )
        form.addRow("Activity detection:", self._activity_cb)

        self._mute_cb = QCheckBox()
        self._mute_cb.setChecked(self._ctx.config.muted)
        self._mute_cb.toggled.connect(self._on_mute)
        form.addRow("Mute:", self._mute_cb)

        port_row = QHBoxLayout()
        self._port_edit = QLineEdit(self._ctx.config.last_port or "")
        port_row.addWidget(self._port_edit)
        save_port = QPushButton("Save port")
        save_port.clicked.connect(self._save_port)
        port_row.addWidget(save_port)
        form.addRow("Serial port:", port_row)

        profiles_path = Path.home() / "AppData" / "Roaming" / "NomaBot" / "activity_profiles.json"
        form.addRow("Activity profiles:", QLabel(str(profiles_path)))

        return w

    def _on_mute(self, muted: bool) -> None:
        self._ctx.config.muted = muted
        self._ctx.state_manager.set_muted(muted)

    def _save_port(self) -> None:
        self._ctx.config.last_port = self._port_edit.text().strip()
