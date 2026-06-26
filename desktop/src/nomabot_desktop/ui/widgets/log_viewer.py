"""Live log tail viewer widget."""

from __future__ import annotations

from pathlib import Path

from PySide6.QtCore import QTimer
from PySide6.QtWidgets import (
    QComboBox,
    QHBoxLayout,
    QPlainTextEdit,
    QPushButton,
    QVBoxLayout,
    QWidget,
)


class LogViewerWidget(QWidget):
    def __init__(self, log_dir: Path, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._log_dir = log_dir
        self._offsets: dict[str, int] = {}
        self._level = "INFO"

        layout = QVBoxLayout(self)
        bar = QHBoxLayout()
        self._file_combo = QComboBox()
        for name in (
            "desktop.log",
            "transport.log",
            "runtime.log",
            "activity.log",
            "scheduler.log",
        ):
            self._file_combo.addItem(name)
        self._file_combo.currentTextChanged.connect(self._reset_view)
        bar.addWidget(self._file_combo)

        self._level_combo = QComboBox()
        self._level_combo.addItems(["DEBUG", "INFO", "WARNING", "ERROR"])
        self._level_combo.setCurrentText("INFO")
        self._level_combo.currentTextChanged.connect(self._set_level)
        bar.addWidget(self._level_combo)

        clear_btn = QPushButton("Clear view")
        clear_btn.clicked.connect(self._clear)
        bar.addWidget(clear_btn)
        layout.addLayout(bar)

        self._text = QPlainTextEdit()
        self._text.setReadOnly(True)
        layout.addWidget(self._text)

        self._timer = QTimer(self)
        self._timer.timeout.connect(self._tail)
        self._timer.start(1000)

    def _set_level(self, level: str) -> None:
        self._level = level

    def _reset_view(self) -> None:
        self._text.clear()
        name = self._file_combo.currentText()
        self._offsets.pop(name, None)

    def _clear(self) -> None:
        self._text.clear()

    def _tail(self) -> None:
        name = self._file_combo.currentText()
        path = self._log_dir / name
        if not path.exists():
            return
        offset = self._offsets.get(name, 0)
        try:
            with path.open(encoding="utf-8", errors="replace") as f:
                f.seek(offset)
                chunk = f.read()
                self._offsets[name] = f.tell()
        except OSError:
            return
        if not chunk:
            return
        levels = {"DEBUG": 0, "INFO": 1, "WARNING": 2, "ERROR": 3}
        min_level = levels.get(self._level, 1)
        for line in chunk.splitlines():
            if not any(lv in line for lv in ("DEBUG", "INFO", "WARNING", "ERROR")):
                self._text.appendPlainText(line)
                continue
            for lv, num in levels.items():
                if f"[{lv}]" in line and num >= min_level:
                    self._text.appendPlainText(line)
                    break
