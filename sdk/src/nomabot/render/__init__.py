"""RenderState + DirtyTracker mirror for emulator and tests."""

from __future__ import annotations

from dataclasses import dataclass
from enum import IntFlag, auto


class DirtyFlags(IntFlag):
    NONE = 0
    HEADER = auto()
    META = auto()
    ENERGY = auto()
    BEHAVIOR = auto()
    MESSAGE = auto()
    CHARACTER = auto()
    BACKGROUND = auto()
    FULL = 0xFF


def quantize_energy(energy: int) -> int:
    if energy <= 0:
        return 0
    if energy >= 100:
        return 100
    return ((energy + 2) // 5) * 5


@dataclass
class RenderState:
    life_mode: str = "work"
    activity: str = "idle"
    emotion: str = "neutral"
    goal: str = "none"
    goal_progress: int = 0
    behavior_id: str = "breathing"
    behavior_label: str = "Breathing..."
    energy: int = 80
    display_energy: int = 80
    curiosity: bool = False
    overlay_text: str = ""

    def __post_init__(self) -> None:
        self.display_energy = quantize_energy(self.energy)


class DirtyTracker:
    def __init__(self) -> None:
        self._last: RenderState | None = None
        self._force_full = False
        self._pending = DirtyFlags.NONE
        self._forced = DirtyFlags.NONE

    def invalidate(self, flags: DirtyFlags = DirtyFlags.FULL) -> None:
        if flags == DirtyFlags.FULL:
            self._last = None
            self._force_full = True
            self._pending = DirtyFlags.NONE
            self._forced = DirtyFlags.NONE

    def force_dirty(self, flags: DirtyFlags) -> None:
        self._forced |= flags

    def note_pending(self, flags: DirtyFlags) -> None:
        self._pending |= flags

    def collect_dirty_flags(self, state: RenderState) -> DirtyFlags:
        if self._last is None or self._force_full:
            self._force_full = False
            self._pending = DirtyFlags.NONE
            self._forced = DirtyFlags.NONE
            return DirtyFlags.FULL

        dirty = self._compute_diff(state) | self._pending | self._forced
        self._pending = DirtyFlags.NONE
        self._forced = DirtyFlags.NONE
        return dirty

    def _compute_diff(self, state: RenderState) -> DirtyFlags:
        dirty = DirtyFlags.NONE
        last = self._last
        assert last is not None
        if state.life_mode != last.life_mode or state.activity != last.activity:
            dirty |= DirtyFlags.HEADER
        if (
            state.emotion != last.emotion
            or state.goal != last.goal
            or state.goal_progress != last.goal_progress
        ):
            dirty |= DirtyFlags.META
        if state.display_energy != last.display_energy:
            dirty |= DirtyFlags.ENERGY
        if (
            state.behavior_id != last.behavior_id
            or state.behavior_label != last.behavior_label
            or state.curiosity != last.curiosity
        ):
            dirty |= DirtyFlags.BEHAVIOR
        if state.overlay_text != last.overlay_text:
            dirty |= DirtyFlags.MESSAGE
        return dirty

    def commit_rendered(self, state: RenderState) -> None:
        self._last = RenderState(
            life_mode=state.life_mode,
            activity=state.activity,
            emotion=state.emotion,
            goal=state.goal,
            goal_progress=state.goal_progress,
            behavior_id=state.behavior_id,
            behavior_label=state.behavior_label,
            energy=state.energy,
            display_energy=state.display_energy,
            curiosity=state.curiosity,
            overlay_text=state.overlay_text,
        )
