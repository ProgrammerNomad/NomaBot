"""Transport adapters for desktop."""

from __future__ import annotations

import time
from collections.abc import Callable

from nomabot.protocol.envelope import Envelope, parse_line
from nomabot.testing import MockDevice
from nomabot.transport.serial import SerialTransport


class TransportAdapter:
    """Wraps SDK transports for DeviceManager."""

    def __init__(self, inner: SerialTransport | MockDevice) -> None:
        self._inner = inner
        self._on_envelope: Callable[[Envelope], None] | None = None

    async def connect(self) -> None:
        await self._inner.connect()

    async def disconnect(self) -> None:
        await self._inner.disconnect()

    async def send(self, data: bytes) -> None:
        await self._inner.send(data)

    def on_envelope(self, callback: Callable[[Envelope], None]) -> None:
        self._on_envelope = callback

        def on_bytes(data: bytes) -> None:
            for line in data.decode("utf-8", errors="replace").split("\n"):
                line = line.strip()
                if not line:
                    continue
                try:
                    env = parse_line(line)
                    if self._on_envelope:
                        self._on_envelope(env)
                except Exception:
                    pass

        self._inner.on_receive(on_bytes)


class EmulatorState:
    """Shared state for emulator UI - mirrors firmware clip playback."""

    def __init__(self, width: int = 170, height: int = 320) -> None:
        self.width = width
        self.height = height
        self.character_id = "nomabot"
        self.animation: str | None = "idle"
        self.message: str | None = None
        self.background = "#1a1a2e"
        self.background_sprite_id = "bg_office"
        self.body_sprite_id = "body_idle_01"
        self.anchor_x = 85
        self.anchor_y = 80
        self._frame_index = 0
        self._frame_start_ms = 0.0

    def reset_clip(self) -> None:
        self._frame_index = 0
        self._frame_start_ms = time.monotonic() * 1000

    def advance_frame(self, assets) -> None:
        from nomabot_desktop.core.asset_registry import AssetRegistry

        if not isinstance(assets, AssetRegistry):
            return
        clip = assets.get_animation(self.character_id, self.animation or "idle")
        if not clip or not clip.get("frames"):
            return
        frames = clip["frames"]
        now_ms = time.monotonic() * 1000
        if self._frame_start_ms == 0:
            self._frame_start_ms = now_ms
        current = frames[self._frame_index % len(frames)]
        self.body_sprite_id = current.get("sprite", self.body_sprite_id)
        duration = current.get("duration_ms", 500)
        if now_ms - self._frame_start_ms >= duration:
            self._frame_start_ms = now_ms
            self._frame_index = (self._frame_index + 1) % len(frames)
            nxt = frames[self._frame_index]
            self.body_sprite_id = nxt.get("sprite", self.body_sprite_id)


class EmulatorTransport(MockDevice):
    """Mock transport that updates emulator state."""

    def __init__(self, state: EmulatorState) -> None:
        super().__init__()
        self.state = state

    async def send(self, data: bytes) -> None:
        line = data.decode("utf-8").strip()
        if line:
            env = parse_line(line)
            if env.cmd == "play_animation" and env.params:
                self.state.animation = env.params.get("animation")
                self.state.reset_clip()
            elif env.cmd == "show_message" and env.params:
                self.state.message = env.params.get("text")
            elif env.cmd == "set_background" and env.params:
                bg = env.params.get("background", "office")
                if bg == "office":
                    self.state.background_sprite_id = "bg_office"
                else:
                    self.state.background_sprite_id = bg
            elif env.cmd == "set_state" and env.params:
                state = env.params.get("state")
                if state == "coding":
                    self.state.animation = "coding"
                elif state == "idle":
                    self.state.animation = "idle"
                self.state.reset_clip()
            elif env.cmd == "load_character" and env.params:
                self.state.character_id = env.params.get("character_id", "nomabot")
                self.state.reset_clip()
        await super().send(data)
