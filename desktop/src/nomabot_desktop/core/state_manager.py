"""Canonical bot state - sole owner of world context (not overlays)."""

from __future__ import annotations

import logging
from collections.abc import Callable, Coroutine
from dataclasses import dataclass
from typing import Any

from nomabot.types import Priority, RenderRequest
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.command_source import CommandSource
from nomabot_desktop.core.context_arbitrator import ContextArbitrator
from nomabot_desktop.core.events import OverlayShow, SchedulerFire, StateRequest
from nomabot_desktop.core.runtime import ACTIVITY_STATES

logger = logging.getLogger("noma.runtime")


@dataclass
class BotState:
    name: str = "idle"
    activity: str = "idle"
    emotion: str | None = None
    life_mode: str | None = "work"
    animation: str | None = None
    active_priority: Priority = Priority.BACKGROUND


class StateManager:
    """Only StateManager changes canonical world context."""

    def __init__(
        self,
        bus: EventBus,
        schedule: Callable[[Coroutine[Any, Any, Any]], None],
    ) -> None:
        self._bus = bus
        self._schedule = schedule
        self._runtime_submit: Callable[[RenderRequest], Coroutine[Any, Any, Any]] | None = None
        self._renderer_submit: Callable[[RenderRequest], Coroutine[Any, Any, Any]] | None = None
        self._state = BotState()
        self._arbitrator = ContextArbitrator()
        self._muted = False
        self._held_priority = Priority.BACKGROUND

        bus.subscribe("state.request", self._on_state_request)
        bus.subscribe("scheduler.fire", self._on_scheduler_fire)

    @property
    def arbitrator(self) -> ContextArbitrator:
        return self._arbitrator

    def bind_runtime(
        self,
        submit_fn: Callable[[RenderRequest], Coroutine[Any, Any, Any]],
        renderer_fn: Callable[[RenderRequest], Coroutine[Any, Any, Any]] | None = None,
    ) -> None:
        self._runtime_submit = submit_fn
        self._renderer_submit = renderer_fn

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
        filtered = self._arbitrator.accept(req)
        if filtered is None:
            logger.debug("Arbitrator rejected %s from %s", req.state, req.source)
            return False
        return self._apply(filtered)

    def _on_state_request(self, payload: StateRequest) -> None:
        self.request(payload)

    def _on_scheduler_fire(self, payload: SchedulerFire) -> None:
        if self._muted:
            return
        if payload.priority < self._held_priority:
            return
        if payload.action == "ShowMessage":
            text = payload.parameters.get("text", "")
            overlay_id = payload.parameters.get("id", f"scheduler_{payload.job_id}")
            self._bus.publish(
                "overlay.show",
                OverlayShow(
                    overlay_id=overlay_id,
                    text=text,
                    priority=payload.priority,
                ),
            )
        elif payload.action == "PlayAnimation":
            anim = payload.parameters.get("animation", "idle")
            self.request(
                StateRequest(
                    state=anim,
                    priority=payload.priority,
                    source=CommandSource.SCHEDULER,
                    animation=anim,
                )
            )
        elif payload.action == "SetEmotion":
            emotion = payload.parameters.get("emotion", "neutral")
            self.request(
                StateRequest(
                    state=self._state.activity,
                    priority=payload.priority,
                    source=CommandSource.SCHEDULER,
                    emotion=emotion,
                )
            )
        elif payload.action == "SetLifeMode":
            mode = payload.parameters.get("mode", "work")
            self.request(
                StateRequest(
                    state=self._state.activity,
                    priority=payload.priority,
                    source=CommandSource.SCHEDULER,
                    life_mode=mode,
                )
            )
        elif payload.action == "TriggerHabit":
            habit = payload.parameters.get("habit", "")
            self.request(
                StateRequest(
                    state=self._state.activity,
                    priority=payload.priority,
                    source=CommandSource.SCHEDULER,
                    habit=habit,
                )
            )

    def _apply(self, req: StateRequest) -> bool:
        self._state.name = req.state
        self._held_priority = req.priority

        if req.emotion is not None:
            self._state.emotion = req.emotion
        if req.life_mode is not None:
            self._state.life_mode = req.life_mode

        if req.state in ACTIVITY_STATES:
            self._state.activity = req.state
        elif req.activity:
            self._state.activity = req.activity

        if req.animation:
            self._state.animation = req.animation

        logger.info(
            "State -> %s activity=%s (priority %s, source %s)",
            req.state,
            self._state.activity,
            req.priority.name,
            req.source,
        )
        self._bus.publish("state.changed", self._state)

        if not self._runtime_submit:
            return True

        request = RenderRequest(
            device_id=req.device_id,
            priority=req.priority,
            activity=self._state.activity if req.state in ACTIVITY_STATES or req.activity else None,
            emotion=req.emotion,
            life_mode=req.life_mode,
            habit=req.habit,
            season=req.season,
            animation=req.animation,
        )

        self._schedule(self._runtime_submit(request))
        if request.animation and self._renderer_submit:
            self._schedule(self._renderer_submit(request))
        return True
