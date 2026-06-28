"""Typed event payloads for EventBus."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any

from nomabot.types import Priority


@dataclass(frozen=True)
class StateRequest:
    """Request a bot state transition (handled by StateManager)."""

    state: str
    priority: Priority = Priority.NORMAL
    source: str = "unknown"
    animation: str | None = None
    activity: str | None = None
    emotion: str | None = None
    life_mode: str | None = None
    habit: str | None = None
    season: str | None = None
    message_text: str | None = None
    device_id: str | None = None


@dataclass(frozen=True)
class SchedulerFire:
    job_id: str
    action: str
    parameters: dict[str, Any] = field(default_factory=dict)
    priority: Priority = Priority.LOW


@dataclass(frozen=True)
class ActivityChanged:
    profile: str
    exe_name: str
    priority: Priority = Priority.NORMAL


@dataclass(frozen=True)
class DeviceConnected:
    device_id: str
    firmware_version: str | None = None
    display_width: int | None = None
    display_height: int | None = None


@dataclass(frozen=True)
class DeviceDisconnected:
    device_id: str
    reason: str = ""
