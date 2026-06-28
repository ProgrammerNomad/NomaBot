"""Arbitrate world-context StateRequests with temporary dev override."""

from __future__ import annotations

import time
from dataclasses import dataclass

from nomabot_desktop.core.command_source import CommandSource, coerce_source, source_rank
from nomabot_desktop.core.events import StateRequest
from nomabot_desktop.core.runtime import ACTIVITY_STATES

OVERRIDE_SECONDS = 30.0

_OVERRIDE_SOURCES = frozenset({CommandSource.DEV_PANEL, CommandSource.USER})


@dataclass
class TemporaryOverride:
    activity: str | None = None
    emotion: str | None = None
    life_mode: str | None = None
    source: CommandSource = CommandSource.DEV_PANEL
    expires_at: float = 0.0

    def active(self, now: float | None = None) -> bool:
        now = now if now is not None else time.monotonic()
        return self.expires_at > now

    def blocks(self, source: CommandSource) -> bool:
        if not self.active():
            return False
        return source_rank(source) < source_rank(self.source)


class ContextArbitrator:
    def __init__(self) -> None:
        self._override = TemporaryOverride()
        self._last_source: CommandSource = CommandSource.BRAIN

    @property
    def override(self) -> TemporaryOverride:
        return self._override

    @property
    def last_source(self) -> CommandSource:
        return self._last_source

    def override_active(self, now: float | None = None) -> bool:
        return self._override.active(now)

    def accept(self, req: StateRequest) -> StateRequest | None:
        source = coerce_source(req.source)
        now = time.monotonic()

        if self._override.active(now) and self._override.blocks(source):
            if source in {
                CommandSource.ACTIVITY,
                CommandSource.SCHEDULER,
                CommandSource.LIFE_MODE,
                CommandSource.ROUTINE,
            }:
                return None

        if source in _OVERRIDE_SOURCES:
            self._apply_override(req, source, now)

        self._last_source = source
        return req

    def _apply_override(self, req: StateRequest, source: CommandSource, now: float) -> None:
        activity = req.activity
        if req.state in ACTIVITY_STATES:
            activity = req.state
        prev = self._override if self._override.active(now) else TemporaryOverride()
        self._override = TemporaryOverride(
            activity=activity or prev.activity,
            emotion=req.emotion if req.emotion is not None else prev.emotion,
            life_mode=req.life_mode if req.life_mode is not None else prev.life_mode,
            source=source,
            expires_at=now + OVERRIDE_SECONDS,
        )
