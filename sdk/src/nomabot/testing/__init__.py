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
        self.firmware_version = "0.4.0"
        self.device_id = "mock-esp32"
        self.display_width = 170
        self.display_height = 320
        self.last_animation: str | None = None
        self.last_message: str | None = None
        self.last_character_id: str | None = None
        self.last_activity: str | None = "idle"
        self.last_emotion: str | None = "neutral"
        self.last_life_mode: str | None = "work"
        self.last_behavior: str | None = "breathing"
        self.next_behavior: str | None = "blink"
        self.render_mode: str = "text"
        self.pack_uuid: str | None = "a6d1e8f0-4c2b-4f91-9c3d-8e1a2b4c6d8e"

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
                    "caps": [
                        "play_animation",
                        "show_message",
                        "set_background",
                        "set_state",
                        "set_activity",
                        "set_emotion",
                        "set_life_mode",
                        "trigger_habit",
                        "set_season",
                        "load_character",
                        "diagnostics",
                    ],
                    "render_mode": self.render_mode,
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
                    "active_animation": self.last_behavior,
                    "activity": self.last_activity,
                    "behavior": self.last_behavior,
                    "fps": 20,
                },
            )
        if cmd == "play_animation" and env.params:
            self.last_animation = env.params.get("animation")
            self.last_behavior = self.last_animation
            return build_response(env.id, "play_animation", data={"ok": True})
        if cmd == "show_message" and env.params:
            self.last_message = env.params.get("text")
            return build_response(env.id, "show_message", data={"ok": True})
        if cmd == "set_background":
            return build_response(env.id, "set_background", data={"ok": True})
        if cmd == "set_state" and env.params:
            self.last_activity = env.params.get("state")
            return build_response(env.id, "set_state", data={"ok": True})
        if cmd == "set_activity" and env.params:
            self.last_activity = env.params.get("activity")
            return build_response(env.id, "set_activity", data={"ok": True, "activity": self.last_activity})
        if cmd == "set_emotion" and env.params:
            self.last_emotion = env.params.get("emotion")
            return build_response(env.id, "set_emotion", data={"ok": True, "emotion": self.last_emotion})
        if cmd == "set_life_mode" and env.params:
            self.last_life_mode = env.params.get("mode")
            return build_response(env.id, "set_life_mode", data={"ok": True, "mode": self.last_life_mode})
        if cmd == "trigger_habit" and env.params:
            return build_response(
                env.id, "trigger_habit", data={"ok": True, "habit": env.params.get("habit")}
            )
        if cmd == "set_season" and env.params:
            return build_response(
                env.id, "set_season", data={"ok": True, "season": env.params.get("season")}
            )
        if cmd == "load_character" and env.params:
            self.last_character_id = env.params.get("character_id")
            return build_response(
                env.id,
                "load_character",
                data={
                    "pack_id": self.last_character_id,
                    "uuid": self.pack_uuid,
                    "version": {"major": 0, "minor": 3, "patch": 1},
                    "display": {
                        "profile": "lilygo_tdisplay_s3",
                        "width": self.display_width,
                        "height": self.display_height,
                    },
                },
            )
        if cmd == "diagnostics":
            return build_response(
                env.id,
                "diagnostics",
                data={
                    "fps": 20,
                    "heap_free": 182304,
                    "psram_free": 6543210,
                    "character_id": self.last_character_id or "nomabot",
                    "uuid": self.pack_uuid,
                    "life_mode": self.last_life_mode,
                    "activity": self.last_activity,
                    "emotion": self.last_emotion,
                    "behavior": self.last_behavior,
                    "render_mode": self.render_mode,
                    "energy": 72,
                    "boredom": 15,
                    "goal": "focus",
                    "goal_progress": 60,
                    "time_in_behavior_sec": 8,
                    "next_behavior": self.next_behavior,
                    "animation": self.last_behavior,
                    "frame": 0,
                    "state": self.last_activity,
                },
            )
        return build_response(
            env.id,
            cmd,
            ok=False,
            error={"code": "unknown_command", "message": f"Unknown command: {cmd}"},
        )


class MockDevice(MockTransport):
    """Alias for MockTransport with device semantics."""

    pass
