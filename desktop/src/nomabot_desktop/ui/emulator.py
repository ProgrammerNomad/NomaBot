"""170x320 device emulator window."""

from __future__ import annotations

import struct
from pathlib import Path

from PySide6.QtCore import Qt, QTimer
from PySide6.QtGui import QColor, QFont, QImage, QPainter
from PySide6.QtWidgets import QLabel, QVBoxLayout, QWidget

from nomabot.brain import Brain
from nomabot.render import DirtyFlags, DirtyTracker, RenderState, quantize_energy
from nomabot.render.background_cache import BackgroundCache
from nomabot.render.scene import Scene, SceneBuilder
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
        self._scene = Scene()
        self._bg_cache = BackgroundCache()
        self._frame_buffer: QImage | None = None
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
            background_sprite_id=self._state.background_sprite_id,
            body_sprite_id=self._state.body_sprite_id,
            clip_frame_index=self._state._frame_index,
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
            self._scene = SceneBuilder.build(
                render_state,
                default_background=self._state.background_sprite_id,
                anchor_x=self._state.anchor_x,
                anchor_y=self._state.anchor_y,
                dirty=dirty,
            )
            self.update()

    def paintEvent(self, event) -> None:  # noqa: N802
        painter = QPainter(self)
        if self._state.render_mode == "text" or not self._assets.get_sprite(
            self._state.character_id, self._state.body_sprite_id
        ):
            self._paint_text_mode(painter, self._render_state, self._dirty)
        else:
            self._paint_sprite_mode(painter, self._scene, self._dirty)
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

    def _load_sprite_image(self, sprite_id: str | None) -> tuple[QImage | None, dict | None]:
        if not sprite_id:
            return None, None
        pack = self._state.character_id
        meta = self._assets.get_sprite(pack, sprite_id)
        path = self._assets.sprite_bin_path(pack, sprite_id)
        if not meta or not path:
            return None, None
        img = _rgb565_to_qimage(path.read_bytes(), meta["width"], meta["height"])
        return img, meta

    def _capture_bg_cache(self, bg_img: QImage, character_node) -> None:
        body_img, body_meta = self._load_sprite_image(character_node.sprite_id)
        if not body_img or not body_meta:
            self._bg_cache.reset()
            return
        draw_x = character_node.x - body_meta["width"] // 2
        draw_y = character_node.y
        pixels: list[int] = []
        for row in range(body_meta["height"]):
            for col in range(body_meta["width"]):
                x = draw_x + col
                y = draw_y + row
                if 0 <= x < bg_img.width() and 0 <= y < bg_img.height():
                    c = bg_img.pixelColor(x, y)
                    pixels.append((c.red() << 8) | (c.green() << 3) | (c.blue() >> 3))
                else:
                    pixels.append(0)
        self._bg_cache.capture_from_background(
            pixels,
            body_meta["width"],
            body_meta["height"],
            0,
            0,
            body_meta["width"],
            body_meta["height"],
        )

    def _paint_sprite_mode(self, painter: QPainter, scene: Scene, dirty: DirtyFlags) -> None:
        if dirty == DirtyFlags.FULL:
            self._frame_buffer = QImage(self.size(), QImage.Format.Format_RGB888)
            self._frame_buffer.fill(QColor("#000000"))

        target = self._frame_buffer
        if target is None:
            target = QImage(self.size(), QImage.Format.Format_RGB888)
            target.fill(QColor("#000000"))
            self._frame_buffer = target

        if scene.background.dirty or dirty == DirtyFlags.FULL:
            bg_img, _ = self._load_sprite_image(scene.background.sprite_id)
            if bg_img:
                buf_painter = QPainter(target)
                buf_painter.drawImage(0, 0, bg_img)
                buf_painter.end()
                self._capture_bg_cache(bg_img, scene.character)
            else:
                target.fill(QColor(self._state.background))

        if scene.character.dirty:
            if not scene.background.dirty and dirty != DirtyFlags.FULL:
                canvas = [
                    [target.pixelColor(x, y).rgb() for x in range(target.width())]
                    for y in range(target.height())
                ]
                self._bg_cache.restore(canvas)
                for y, row in enumerate(canvas):
                    for x, px in enumerate(row):
                        c = QColor(px)
                        target.setPixelColor(x, y, c)

            body_img, body_meta = self._load_sprite_image(scene.character.sprite_id)
            if body_img and body_meta:
                ax = scene.character.x - body_meta["width"] // 2
                ay = scene.character.y
                buf_painter = QPainter(target)
                buf_painter.drawImage(ax, ay, body_img)
                buf_painter.end()

        buf_painter = QPainter(target)
        buf_painter.setPen(Qt.GlobalColor.white)
        buf_painter.setFont(QFont("Segoe UI", 8))

        if scene.hud.dirty and scene.hud.visible and scene.hud.text:
            buf_painter.fillRect(0, 0, self.width(), 18, QColor("#000000"))
            buf_painter.drawText(scene.hud.x, scene.hud.y + 6, scene.hud.text)

        if scene.speech_bubble.dirty and scene.speech_bubble.visible and scene.speech_bubble.text:
            band_y = self.height() - 24
            buf_painter.fillRect(0, band_y, self.width(), 24, QColor("#000000"))
            buf_painter.drawText(
                scene.speech_bubble.x,
                self.height() - 12,
                scene.speech_bubble.text[:40],
            )
        buf_painter.end()

        painter.drawImage(0, 0, target)


class EmulatorWindow(QWidget):
    def __init__(self, state: EmulatorState, assets: AssetRegistry) -> None:
        super().__init__()
        self.setWindowTitle("NomaBot Emulator")
        layout = QVBoxLayout(self)
        layout.addWidget(QLabel("LILYGO T-Display S3 - 170×320 (Tiny World Renderer)"))
        layout.addWidget(EmulatorCanvas(state, assets))
        self.resize(200, 400)
