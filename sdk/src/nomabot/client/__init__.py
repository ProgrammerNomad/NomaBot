"""Transport-agnostic device client."""

from __future__ import annotations

import asyncio
from collections.abc import Callable
from typing import Any, Protocol

from nomabot.protocol.commands import (
    HelloParams,
    PlayAnimationParams,
    SetBackgroundParams,
    SetStateParams,
    ShowMessageParams,
    build_command,
)
from nomabot.protocol.envelope import Envelope, parse_line


class Transport(Protocol):
    async def connect(self) -> None: ...
    async def disconnect(self) -> None: ...
    async def send(self, data: bytes) -> None: ...
    def on_receive(self, callback: Callable[[bytes], None]) -> None: ...


class NomaClient:
    """Low-level protocol client over any Transport."""

    def __init__(self, transport: Transport) -> None:
        self._transport = transport
        self._buffer = ""
        self._responses: dict[str, asyncio.Future[Envelope]] = {}
        self._connected = False
        self._transport.on_receive(self._on_bytes)

    async def connect(self) -> Envelope:
        await self._transport.connect()
        self._connected = True
        self._transport.on_receive(self._on_bytes)
        return await self.send_command(
            build_command("hello", HelloParams(client_version="0.1.0"))
        )

    async def disconnect(self) -> None:
        self._connected = False
        await self._transport.disconnect()

    def _on_bytes(self, data: bytes) -> None:
        self._buffer += data.decode("utf-8", errors="replace")
        while "\n" in self._buffer:
            line, self._buffer = self._buffer.split("\n", 1)
            if not line.strip():
                continue
            try:
                env = parse_line(line)
            except Exception:
                continue
            if env.id in self._responses:
                fut = self._responses.pop(env.id)
                if not fut.done():
                    fut.set_result(env)

    async def send_command(self, envelope: Envelope, timeout: float = 5.0) -> Envelope:
        loop = asyncio.get_event_loop()
        fut: asyncio.Future[Envelope] = loop.create_future()
        self._responses[envelope.id] = fut
        await self._transport.send(envelope.to_json_line().encode("utf-8"))
        try:
            return await asyncio.wait_for(fut, timeout=timeout)
        except TimeoutError:
            self._responses.pop(envelope.id, None)
            raise

    async def send_fire_and_forget(self, envelope: Envelope) -> None:
        await self._transport.send(envelope.to_json_line().encode("utf-8"))

    async def ping(self) -> Envelope:
        return await self.send_command(build_command("ping"))

    async def play_animation(self, animation: str, *, loop: bool = True) -> Envelope:
        return await self.send_command(
            build_command("play_animation", PlayAnimationParams(animation=animation, loop=loop))
        )

    async def say(self, text: str, *, style: str = "speech") -> Envelope:
        return await self.send_command(
            build_command("show_message", ShowMessageParams(text=text, style=style))
        )

    async def set_background(self, background: str) -> Envelope:
        return await self.send_command(
            build_command("set_background", SetBackgroundParams(background=background))
        )

    async def set_state(self, state: str) -> Envelope:
        return await self.send_command(
            build_command("set_state", SetStateParams(state=state))
        )

    async def get_status(self) -> Envelope:
        return await self.send_command(build_command("get_status"))


class NomaBot:
    """High-level developer API."""

    def __init__(self, client: NomaClient) -> None:
        self._client = client

    @classmethod
    async def connect_serial(cls, port: str, baud: int = 115200) -> NomaBot:
        from nomabot.transport.serial import SerialTransport

        transport = SerialTransport(port, baud)
        client = NomaClient(transport)
        await client.connect()
        return cls(client)

    @classmethod
    def from_mock(cls, mock: Any) -> NomaBot:
        return cls(NomaClient(mock))

    async def play_animation(self, animation: str, *, loop: bool = True) -> Envelope:
        return await self._client.play_animation(animation, loop=loop)

    async def say(self, text: str, *, style: str = "speech") -> Envelope:
        return await self._client.say(text, style=style)

    async def set_background(self, background: str) -> Envelope:
        return await self._client.set_background(background)

    async def set_state(self, state: str) -> Envelope:
        return await self._client.set_state(state)

    async def ping(self) -> Envelope:
        return await self._client.ping()

    async def disconnect(self) -> None:
        await self._client.disconnect()
