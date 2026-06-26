"""Canonical bot state - sole owner of current state."""

from __future__ import annotations

import logging
from collections.abc import Callable, Coroutine
from dataclasses import dataclass
from typing import Any

from nomabot.types import MessageSpec, Priority, RenderRequest
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.events import SchedulerFire, StateRequest

logger = logging.getLogger("noma.runtime")


@dataclass
class BotState:
    name: str = "idle"
    animation: str | None = "idle"
    message_text: str | None = None
    active_priority: Priority = Priority.BACKGROUND


class StateManager:
    """Only StateManager changes canonical bot state."""

    def __init__(
        self,
        bus: EventBus,
        schedule: Callable[[Coroutine[Any, Any, Any]], None],
    ) -> None:
        self._bus = bus
        self._schedule = schedule
        self._runtime_submit: Callable[[RenderRequest], Coroutine[Any, Any, Any]] | None = None
        self._state = BotState()
        self._muted = False
        self._held_priority = Priority.BACKGROUND

        bus.subscribe("state.request", self._on_state_request)
        bus.subscribe("scheduler.fire", self._on_scheduler_fire)

    def bind_runtime(self, submit_fn: Callable[[RenderRequest], Coroutine[Any, Any, Any]]) -> None:
        self._runtime_submit = submit_fn

    @property
    def state(self) -> BotState:
        return self._state

    @property
    def muted(self) -> bool:
        return self._muted

    def set_muted(self, muted: bool) -> None:
        self._muted = muted

    def request(self, req: StateRequest) -> bool:
        if self._muted and req.priority < Priority.CRITICAL:
            return False
        if req.priority < self._held_priority and req.state != self._state.name:
            logger.debug(
                "Rejected state %s (priority %s < held %s)",
                req.state,
                req.priority,
                self._held_priority,
            )
            return False
        return self._apply(req)

    def _on_state_request(self, payload: StateRequest) -> None:
        self.request(payload)

    def _on_scheduler_fire(self, payload: SchedulerFire) -> None:
        if self._muted:
            return
        if payload.priority < self._held_priority:
            return
        if payload.action == "ShowMessage":
            text = payload.parameters.get("text", "")
            self._apply(
                StateRequest(
                    state="message_active",
                    priority=payload.priority,
                    source=f"scheduler:{payload.job_id}",
                    message_text=text,
                )
            )
        elif payload.action == "PlayAnimation":
            anim = payload.parameters.get("animation", "idle")
            self._apply(
                StateRequest(
                    state=anim,
                    priority=payload.priority,
                    source=f"scheduler:{payload.job_id}",
                    animation=anim,
                )
            )

    def _apply(self, req: StateRequest) -> bool:
        self._state.name = req.state
        self._held_priority = req.priority
        if req.animation:
            self._state.animation = req.animation
        elif req.state in ("idle", "coding", "gaming"):
            self._state.animation = req.state
        if req.message_text is not None:
            self._state.message_text = req.message_text

        logger.info(
            "State -> %s (priority %s, source %s)",
            req.state,
            req.priority.name,
            req.source,
        )
        self._bus.publish("state.changed", self._state)

        if not self._runtime_submit:
            return True

        if req.message_text:
            request = RenderRequest(
                device_id=req.device_id,
                animation=self._state.animation,
                message=MessageSpec(text=req.message_text),
                priority=req.priority,
            )
        else:
            request = RenderRequest(
                device_id=req.device_id,
                animation=self._state.animation,
                priority=req.priority,
            )
        self._schedule(self._runtime_submit(request))
        return True
