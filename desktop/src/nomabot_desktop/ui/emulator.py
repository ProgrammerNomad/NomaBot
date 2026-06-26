"""170x320 device emulator window."""

from __future__ import annotations

import struct

from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QColor, QFont, QImage, QPainter
from PySide6.QtWidgets import QLabel, QVBoxLayout, QWidget

from nomabot_desktop.core.asset_registry import AssetRegistry
from nomabot_desktop.transport import EmulatorState


def _rgb565_to_qimage(data: bytes, width: int, height: int) -> QImage:
    pixels = []
    for i in range(0, len(data), 2):
        if i + 1 >= len(data):
            break
        (px,) = struct.unpack_from("<H", data, i)
        r = ((px >> 11) & 0x1F) << 3
        g = ((px >> 5) & 0x3F) << 2
        b = (px & 0x1F) << 3
        pixels.extend([r, g, b])
    img = QImage(width, height, QImage.Format.Format_RGB888)
    if len(pixels) >= width * height * 3:
        img.fill(Qt.GlobalColor.black)
        for y in range(height):
            for x in range(width):
                idx = (y * width + x) * 3
                img.setPixelColor(x, y, QColor(pixels[idx], pixels[idx + 1], pixels[idx + 2]))
    return img


class EmulatorCanvas(QWidget):
    def __init__(
        self,
        state: EmulatorState,
        assets: AssetRegistry,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self._state = state
        self._assets = assets
        self.setFixedSize(state.width, state.height)

        self._timer = QTimer(self)
        self._timer.timeout.connect(self._on_tick)
        self._timer.start(50)

    def _on_tick(self) -> None:
        self._state.advance_frame(self._assets)
        self.update()

    def paintEvent(self, event) -> None:  # noqa: N802
        painter = QPainter(self)
        pack = self._state.character_id

        bg_id = self._state.background_sprite_id
        bg_meta = self._assets.get_sprite(pack, bg_id)
        bg_path = self._assets.sprite_bin_path(pack, bg_id)
        if bg_meta and bg_path:
            bg_img = _rgb565_to_qimage(bg_path.read_bytes(), bg_meta["width"], bg_meta["height"])
            painter.drawImage(0, 0, bg_img)
        else:
            painter.fillRect(self.rect(), QColor(self._state.background))

        body_id = self._state.body_sprite_id
        body_meta = self._assets.get_sprite(pack, body_id)
        body_path = self._assets.sprite_bin_path(pack, body_id)
        if body_meta and body_path:
            body_img = _rgb565_to_qimage(
                body_path.read_bytes(), body_meta["width"], body_meta["height"]
            )
            ax, ay = self._state.anchor_x, self._state.anchor_y
            x = ax - body_meta["width"] // 2
            painter.drawImage(x, ay, body_img)

        painter.setPen(Qt.GlobalColor.white)
        painter.setFont(QFont("Segoe UI", 8))
        painter.drawText(4, 14, f"anim: {self._state.animation or 'none'}")

        if self._state.message:
            painter.setFont(QFont("Segoe UI", 9))
            painter.drawText(8, self.height() - 40, self._state.message[:40])

        painter.end()


class EmulatorWindow(QWidget):
    def __init__(self, state: EmulatorState, assets: AssetRegistry) -> None:
        super().__init__()
        self.setWindowTitle("NomaBot Emulator")
        layout = QVBoxLayout(self)
        layout.addWidget(QLabel("LILYGO T-Display S3 - 170×320"))
        layout.addWidget(EmulatorCanvas(state, assets))
        self.resize(200, 400)
