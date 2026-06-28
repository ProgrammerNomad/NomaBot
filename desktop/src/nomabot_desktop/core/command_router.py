"""Route commands to Context, Overlay, or Renderer destinations."""

from __future__ import annotations

from collections.abc import Callable, Coroutine
from typing import Any

from nomabot.types import Priority, RenderRequest
from nomabot_desktop.core.events import OverlayShow, StateRequest
from nomabot_desktop.core.overlay_service import OverlayService
from nomabot_desktop.core.runtime import NomaRuntime
from nomabot_desktop.core.state_manager import StateManager


class CommandRouter:
    """Single entry for dev/tools: no special-case priority hacks."""

    def __init__(
        self,
        state_manager: StateManager,
        overlay: OverlayService,
        runtime: NomaRuntime,
        schedule: Callable[[Coroutine[Any, Any, Any]], None],
    ) -> None:
        self._state = state_manager
        self._overlay = overlay
        self._runtime = runtime
        self._schedule = schedule

    def context(self, req: StateRequest) -> bool:
        return self._state.request(req)

    def overlay(
        self,
        *,
        overlay_id: str,
        text: str,
        priority: Priority = Priority.NORMAL,
        duration_ms: int = 5000,
        style: str = "speech",
        device_id: str | None = None,
    ) -> None:
        self._overlay.show(
            overlay_id=overlay_id,
            text=text,
            priority=priority,
            duration_ms=duration_ms,
            style=style,
            device_id=device_id,
        )

    def overlay_event(self, payload: OverlayShow) -> None:
        self._overlay.show(
            overlay_id=payload.overlay_id,
            text=payload.text,
            priority=payload.priority,
            duration_ms=payload.duration_ms,
            style=payload.style,
            device_id=payload.device_id,
        )

    def renderer(self, request: RenderRequest) -> None:
        self._schedule(self._runtime.submit_renderer(request))
