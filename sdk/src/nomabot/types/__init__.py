"""Shared types for desktop and SDK."""

from __future__ import annotations

from enum import IntEnum

from pydantic import BaseModel


class Priority(IntEnum):
    CRITICAL = 4
    HIGH = 3
    NORMAL = 2
    LOW = 1
    BACKGROUND = 0


class MessageSpec(BaseModel):
    text: str
    style: str = "speech"
    duration_ms: int = 5000


class RenderRequest(BaseModel):
    """High-level render intent submitted to Noma Runtime."""

    device_id: str | None = None
    animation: str | None = None
    accessory: str | None = None
    background: str | None = None
    message: MessageSpec | None = None
    state: str | None = None
    activity: str | None = None
    emotion: str | None = None
    life_mode: str | None = None
    priority: Priority = Priority.NORMAL
