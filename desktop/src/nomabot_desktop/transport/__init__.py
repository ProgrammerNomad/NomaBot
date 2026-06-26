"""Transport adapters for desktop."""

from __future__ import annotations

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
    """Shared state for emulator UI."""

    def __init__(self, width: int = 170, height: int = 320) -> None:
        self.width = width
        self.height = height
        self.animation: str | None = "idle"
        self.message: str | None = None
        self.background: str = "#1a1a2e"


class EmulatorTransport(MockDevice):
    """Mock transport that updates emulator state."""

    def __init__(self, state: EmulatorState) -> None:
        super().__init__()
        self.state = state

    async def send(self, data: bytes) -> None:
        line = data.decode("utf-8").strip()
        if line:
            from nomabot.protocol.envelope import parse_line

            env = parse_line(line)
            if env.cmd == "play_animation" and env.params:
                self.state.animation = env.params.get("animation")
            elif env.cmd == "show_message" and env.params:
                self.state.message = env.params.get("text")
            elif env.cmd == "set_background" and env.params:
                self.state.background = env.params.get("background", "#1a1a2e")
        await super().send(data)
