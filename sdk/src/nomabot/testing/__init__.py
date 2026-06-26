"""Mock transport and device for testing without hardware."""

from __future__ import annotations

from collections.abc import Callable

from nomabot.protocol.commands import build_response
from nomabot.protocol.envelope import Envelope, parse_line


class MockTransport:
    """Records outbound commands and auto-responds to hello/ping."""

    def __init__(self) -> None:
        self.sent: list[bytes] = []
        self.sent_envelopes: list[Envelope] = []
        self._callback: Callable[[bytes], None] | None = None
        self._connected = False
        self.firmware_version = "0.1.0"
        self.device_id = "mock-esp32"
        self.display_width = 170
        self.display_height = 320
        self.last_animation: str | None = None
        self.last_message: str | None = None

    async def connect(self) -> None:
        self._connected = True

    async def disconnect(self) -> None:
        self._connected = False

    def on_receive(self, callback: Callable[[bytes], None]) -> None:
        self._callback = callback

    async def send(self, data: bytes) -> None:
        self.sent.append(data)
        line = data.decode("utf-8").strip()
        if not line:
            return
        env = parse_line(line)
        self.sent_envelopes.append(env)
        response = self._handle_command(env)
        if response and self._callback:
            self._callback(response.to_json_line().encode("utf-8"))

    def _handle_command(self, env: Envelope) -> Envelope | None:
        cmd = env.cmd or ""
        if cmd == "hello":
            return build_response(
                env.id,
                "hello",
                data={
                    "protocol": 1,
                    "firmware": self.firmware_version,
                    "firmware_version": self.firmware_version,
                    "board": "MOCK_BOARD",
                    "serial": "MOCK-SERIAL-001",
                    "device_id": self.device_id,
                    "display": {
                        "width": self.display_width,
                        "height": self.display_height,
                        "fps": 20,
                    },
                    "caps": ["play_animation", "show_message", "set_background", "set_state"],
                },
            )
        if cmd == "ping":
            return build_response(env.id, "ping", data={"pong": True})
        if cmd == "get_status":
            return build_response(
                env.id,
                "get_status",
                data={
                    "firmware_version": self.firmware_version,
                    "active_animation": self.last_animation,
                    "fps": 20,
                },
            )
        if cmd == "play_animation" and env.params:
            self.last_animation = env.params.get("animation")
            return build_response(env.id, "play_animation", data={"ok": True})
        if cmd == "show_message" and env.params:
            self.last_message = env.params.get("text")
            return build_response(env.id, "show_message", data={"ok": True})
        if cmd == "set_background":
            return build_response(env.id, "set_background", data={"ok": True})
        if cmd == "set_state":
            return build_response(env.id, "set_state", data={"ok": True})
        return build_response(
            env.id,
            cmd,
            ok=False,
            error={"code": "unknown_command", "message": f"Unknown command: {cmd}"},
        )


class MockDevice(MockTransport):
    """Alias for MockTransport with device semantics."""

    pass
