"""170x320 device emulator window."""

from __future__ import annotations

from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QColor, QFont, QPainter
from PySide6.QtWidgets import QLabel, QVBoxLayout, QWidget

from nomabot_desktop.transport import EmulatorState


class EmulatorCanvas(QWidget):
    def __init__(self, state: EmulatorState, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._state = state
        self.setFixedSize(state.width, state.height)
        self.setWindowTitle("NomaBot Emulator (170×320)")

        self._timer = QTimer(self)
        self._timer.timeout.connect(self.update)
        self._timer.start(100)
        self._blink = False

    def paintEvent(self, event) -> None:  # noqa: N802
        painter = QPainter(self)
        painter.fillRect(self.rect(), QColor(self._state.background))

        # Placeholder character: colored rect + stick figure style
        self._blink = not self._blink
        body_color = (
            QColor("#4cc9f0")
            if self._blink or self._state.animation != "idle"
            else QColor("#4361ee")
        )
        cx, cy = self.width() // 2, self.height() // 2
        painter.fillRect(cx - 30, cy - 40, 60, 80, body_color)

        # Head
        painter.setBrush(body_color)
        painter.drawEllipse(cx - 20, cy - 70, 40, 40)

        # Animation label
        painter.setPen(Qt.GlobalColor.white)
        painter.setFont(QFont("Segoe UI", 8))
        painter.drawText(4, 14, f"anim: {self._state.animation or 'none'}")

        # Speech bubble
        if self._state.message:
            painter.setPen(Qt.GlobalColor.white)
            painter.setFont(QFont("Segoe UI", 9))
            painter.drawText(8, self.height() - 40, self._state.message[:40])

        painter.end()


class EmulatorWindow(QWidget):
    def __init__(self, state: EmulatorState) -> None:
        super().__init__()
        self.setWindowTitle("NomaBot Emulator")
        layout = QVBoxLayout(self)
        layout.addWidget(QLabel("LILYGO T-Display S3 — 170×320"))
        layout.addWidget(EmulatorCanvas(state))
        self.resize(200, 400)
