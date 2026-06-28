"""170x320 device emulator window."""

from __future__ import annotations

import struct
from pathlib import Path

from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QColor, QFont, QImage, QPainter
from PySide6.QtWidgets import QLabel, QVBoxLayout, QWidget

from nomabot.brain import Brain
from nomabot.render import DirtyFlags, DirtyTracker, RenderState, quantize_energy
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


def _behavior_yaml() -> Path:
    return Path(__file__).resolve().parents[4] / "assets" / "characters" / "nomabot" / "behavior.yaml"


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
        self._brain = Brain.from_yaml(_behavior_yaml())
        self._brain.set_life_mode(state.life_mode)
        self._brain.set_activity(state.activity)
        self._brain.set_emotion(state.emotion or "neutral")
        self._tracker = DirtyTracker()
        self._dirty = DirtyFlags.FULL
        self._render_state = RenderState()
        self.setFixedSize(state.width, state.height)
        self.setAutoFillBackground(True)

        self._timer = QTimer(self)
        self._timer.timeout.connect(self._on_tick)
        self._timer.start(50)

    def _sync_from_state(self) -> None:
        if self._brain.life_mode != (self._state.life_mode or "work"):
            self._brain.set_life_mode(self._state.life_mode or "work")
        if self._brain.activity != (self._state.activity or "idle"):
            self._brain.set_activity(self._state.activity or "idle")
        if self._brain.emotion != (self._state.emotion or "neutral"):
            self._brain.set_emotion(self._state.emotion or "neutral")
        if self._state.season:
            self._brain.set_season(self._state.season)

    def _build_render_state(self) -> RenderState:
        energy = self._brain.energy
        return RenderState(
            life_mode=self._state.life_mode or "work",
            activity=self._state.activity or "idle",
            emotion=self._state.emotion or "neutral",
            goal=self._brain.goal,
            goal_progress=self._brain.goal_progress,
            behavior_id=self._brain.behavior_id,
            behavior_label=self._brain.behavior_label,
            energy=energy,
            display_energy=quantize_energy(energy),
            curiosity=self._brain.curiosity_active,
            overlay_text=self._state.message or "",
        )

    def _on_tick(self) -> None:
        self._sync_from_state()
        self._brain.tick()
        self._state.behavior = self._brain.behavior_id
        self._state.behavior_label = self._brain.behavior_label
        self._state.goal = self._brain.goal
        self._state.goal_progress = self._brain.goal_progress
        self._state.energy = self._brain.energy
        self._state.curiosity_active = self._brain.curiosity_active
        if self._state.render_mode != "text":
            self._state.advance_frame(self._assets)

        render_state = self._build_render_state()
        dirty = self._tracker.collect_dirty_flags(render_state)
        if dirty != DirtyFlags.NONE:
            self._render_state = render_state
            self._dirty = dirty
            self.update()

    def paintEvent(self, event) -> None:  # noqa: N802
        painter = QPainter(self)
        if self._state.render_mode == "text" or not self._assets.get_sprite(
            self._state.character_id, self._state.body_sprite_id
        ):
            self._paint_text_mode(painter, self._render_state, self._dirty)
        else:
            self._paint_sprite_mode(painter)
        if self._dirty != DirtyFlags.NONE:
            self._tracker.commit_rendered(self._render_state)
            self._dirty = DirtyFlags.NONE
        painter.end()

    @staticmethod
    def _clear_band(painter: QPainter, y: int, h: int, width: int) -> None:
        painter.fillRect(0, y, width, h, QColor("#000000"))

    def _paint_text_mode(self, painter: QPainter, state: RenderState, dirty: DirtyFlags) -> None:
        w = self.width()
        if dirty == DirtyFlags.FULL:
            painter.fillRect(self.rect(), QColor("#000000"))

        painter.setFont(QFont("Segoe UI", 8))

        if dirty == DirtyFlags.FULL or DirtyFlags.HEADER in dirty:
            self._clear_band(painter, 0, 18, w)
            painter.setPen(Qt.GlobalColor.white)
            header = f"{state.life_mode} · {state.activity}"
            painter.drawText(4, 14, header)

        if dirty == DirtyFlags.FULL or DirtyFlags.META in dirty:
            self._clear_band(painter, 18, 18, w)
            if state.goal and state.goal != "none":
                meta = f"{state.emotion} · {state.goal} · {state.goal_progress}%"
                painter.setPen(QColor("#AD55AD"))
                painter.drawText(4, 30, meta)
            elif state.emotion != "neutral":
                painter.setPen(QColor("#AD55AD"))
                painter.drawText(4, 30, state.emotion)

        if dirty == DirtyFlags.FULL or DirtyFlags.ENERGY in dirty:
            self._clear_band(painter, 34, 18, w)
            painter.setPen(QColor("#7BEF7B"))
            painter.drawText(4, 46, f"Energy: {state.display_energy}")

        if dirty == DirtyFlags.FULL or DirtyFlags.BEHAVIOR in dirty:
            self._clear_band(painter, 50, 18, w)
            if state.curiosity:
                painter.setPen(QColor("#FD20FD"))
                painter.drawText(4, 62, "I wonder...")
            else:
                painter.setPen(Qt.GlobalColor.white)
                painter.drawText(4, 62, state.behavior_label)

        if dirty == DirtyFlags.FULL or DirtyFlags.MESSAGE in dirty:
            overlay_y = self.height() - 28
            self._clear_band(painter, overlay_y, 28, w)
            if state.overlay_text:
                painter.setPen(Qt.GlobalColor.white)
                painter.drawText(4, self.height() - 12, state.overlay_text[:36])

    def _paint_sprite_mode(self, painter: QPainter) -> None:
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


class EmulatorWindow(QWidget):
    def __init__(self, state: EmulatorState, assets: AssetRegistry) -> None:
        super().__init__()
        self.setWindowTitle("NomaBot Emulator")
        layout = QVBoxLayout(self)
        layout.addWidget(QLabel("LILYGO T-Display S3 - 170×320 (text brain)"))
        layout.addWidget(EmulatorCanvas(state, assets))
        self.resize(200, 400)
