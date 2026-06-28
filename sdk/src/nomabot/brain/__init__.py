"""Unified Brain mirror for tests and emulator preview."""

from __future__ import annotations

import random
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import yaml

from nomabot.behavior import BehaviorEngine, Personality  # noqa: F401 — re-export


@dataclass
class Brain(BehaviorEngine):
    """Python mirror of firmware Brain (energy, boredom, goals, habits, curiosity)."""

    season: str = "spring"
    goal: str = "none"
    goal_progress: int = 0
    energy: int = 80
    boredom: int = 0
    curiosity_active: bool = False
    likes: list[str] = field(default_factory=list)
    _pick_mode: str = "weighted"
    _sequence: list[str] = field(default_factory=list)
    _sequence_index: int = 0
    _behavior_start: float = 0.0
    _behavior_duration: float = 5.0
    _last_curiosity: float = 0.0
    _last_tick: float = 0.0

    @classmethod
    def from_yaml(cls, path: Path) -> Brain:
        data = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
        brain = cls()
        brain._tables = data
        if "personality" in data:
            brain.personality = Personality(**data["personality"])
        if "likes" in data:
            brain.likes = list(data["likes"])
        brain.energy = brain.personality.energy
        brain.runtime.energy = brain.personality.energy
        brain.runtime.coffee_love = brain.personality.coffee_love
        return brain

    def set_season(self, season: str) -> None:
        self.season = season or "spring"

    def trigger_habit(self, habit_id: str) -> None:
        sequences = {
            "morning": ["stretch", "coffee", "look_around"],
            "tea_break": ["coffee"],
            "evening": ["look_around", "read", "smile"],
            "dream": ["dream", "travel", "bike", "coffee"],
        }
        if habit_id == "tea_break":
            self.set_emotion("happy")
            self.apply_behavior("coffee")
            return
        steps = sequences.get(habit_id)
        if steps:
            self._enter_sequence(steps, habit_id)

    def set_life_mode(self, mode: str) -> None:
        super().set_life_mode(mode)
        if mode == "night":
            self.goal = "wind_down"

    def set_activity(self, activity: str) -> None:
        if self.activity != activity:
            self.activity = activity
            self.runtime.idle_minutes = 0
            if activity == "coding" and self.emotion != "frustrated":
                self.goal = "focus"
                self.goal_progress = 0
            elif activity == "idle":
                self.goal = "none"
                self.goal_progress = 0
            self.pick_behavior(force=True)

    def set_emotion(self, emotion: str) -> None:
        self.emotion = emotion or "neutral"
        if emotion == "frustrated":
            self.goal = "recover"
            self.goal_progress = 0
            self._enter_sequence(["think", "sigh", "coffee", "typing_slow"], "recover")
        else:
            self.pick_behavior(force=True)

    def apply_behavior(self, behavior_id: str) -> None:
        if behavior_id == self.behavior_id:
            self.boredom = min(100, self.boredom + 12)
        else:
            self.boredom = max(0, self.boredom - 8)
        self.behavior_id = behavior_id
        self.behavior_label = self._label_for(behavior_id)
        self._behavior_start = time.monotonic()
        self._behavior_duration = random.uniform(3.0, 8.0)

    def _label_for(self, behavior_id: str) -> str:
        for table in (self._activity_behaviors(), self._emotion_behaviors() or []):
            for entry in table:
                if entry.get("id") == behavior_id:
                    return str(entry.get("label", behavior_id))
        return behavior_id.replace("_", " ").title()

    def _enter_sequence(self, steps: list[str], goal_name: str) -> None:
        self._pick_mode = "sequence"
        self._sequence = steps
        self._sequence_index = 0
        self.goal = goal_name
        if steps:
            self.apply_behavior(steps[0])

    def _advance_sequence(self) -> None:
        if self._pick_mode != "sequence" or not self._sequence:
            return
        self._sequence_index += 1
        if self._sequence_index >= len(self._sequence):
            self._pick_mode = "weighted"
            self._sequence = []
            self._sequence_index = 0
            self.goal = "focus" if self.goal == "recover" else "none"
            self.pick_behavior(force=True)
            return
        self.apply_behavior(self._sequence[self._sequence_index])

    def _scaled_weight(self, entry: dict[str, Any]) -> int:
        weight = super()._scaled_weight(entry)
        behavior_id = entry.get("id", "")
        if self.boredom > 40 and behavior_id in {"coffee", "stretch", "look_around"}:
            weight *= 2
        if self.curiosity_active and behavior_id in {"look_around", "read"}:
            weight *= 3
        if self.season == "monsoon" and behavior_id in {"look_around", "read"}:
            weight = weight * 3 // 2
        entry_likes = entry.get("likes") or []
        if entry_likes and any(like in self.likes for like in entry_likes):
            weight = weight * 3 // 2
        return max(1, weight)

    def _update_energy(self) -> None:
        if self.activity == "coding":
            self.energy = max(5, self.energy - 1)
        elif self.behavior_id in {"coffee", "stretch", "sleep"}:
            self.energy = min(100, self.energy + 8)
        self.runtime.energy = self.energy

    def _update_curiosity(self, now: float) -> None:
        if self._last_curiosity == 0:
            self._last_curiosity = now
        if now - self._last_curiosity > 45 and random.randint(0, 99) < self.personality.curiosity // 2:
            self.curiosity_active = True
            self._last_curiosity = now
            self.behavior_label = "I wonder..."
        elif self.curiosity_active and now - self._behavior_start > 8:
            self.curiosity_active = False

    def _update_goal(self) -> None:
        if self.goal != "focus" or self.activity != "coding":
            return
        if self.behavior_id in {"typing", "think"}:
            self.goal_progress = min(100, self.goal_progress + 8)
        if self.goal_progress >= 100:
            self.apply_behavior("celebrate")
            self.goal_progress = 0
            self.goal = "none"

    def tick(self, now: float | None = None) -> None:
        """Advance brain state (call ~20 Hz from emulator)."""
        now = now if now is not None else time.monotonic()
        if self._last_tick == 0:
            self._last_tick = now
        if self._pick_mode == "sequence":
            if now - self._behavior_start >= self._behavior_duration:
                self._advance_sequence()
            self._update_goal()
            return
        if self._behavior_start == 0:
            self.pick_behavior(force=True)
            self._behavior_start = now
        elif now - self._behavior_start >= self._behavior_duration:
            self.pick_behavior(force=True)
            self._behavior_start = now
        if now - self._last_tick >= 1.0:
            self._update_energy()
            self._last_tick = now
        self._update_curiosity(now)
        self._update_goal()

    def diagnostics(self) -> dict[str, Any]:
        return {
            "life_mode": self.life_mode,
            "activity": self.activity,
            "emotion": self.emotion,
            "energy": self.energy,
            "boredom": self.boredom,
            "goal": self.goal,
            "goal_progress": self.goal_progress,
            "behavior": self.behavior_id,
            "render_mode": "text",
        }
