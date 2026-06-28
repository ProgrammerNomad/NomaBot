"""Behavior engine mirror for tests and emulator preview."""

from __future__ import annotations

import random
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import yaml


@dataclass
class Personality:
    energy: int = 80
    coffee_love: int = 90
    curiosity: int = 70
    sleepiness: int = 20
    optimism: int = 90
    patience: int = 40
    playfulness: int = 60


@dataclass
class RuntimeContext:
    energy: int = 80
    coffee_love: int = 90
    idle_minutes: int = 0


@dataclass
class BehaviorEngine:
    """Python mirror of firmware behavior selection logic."""

    life_mode: str = "work"
    activity: str = "idle"
    emotion: str = "neutral"
    behavior_id: str = "breathing"
    behavior_label: str = "Breathing..."
    next_behavior_id: str = "blink"
    personality: Personality = field(default_factory=Personality)
    runtime: RuntimeContext = field(default_factory=RuntimeContext)
    _tables: dict[str, Any] = field(default_factory=dict, repr=False)

    @classmethod
    def from_yaml(cls, path: Path) -> BehaviorEngine:
        data = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
        engine = cls()
        engine._tables = data
        if "personality" in data:
            engine.personality = Personality(**data["personality"])
        engine.runtime.coffee_love = engine.personality.coffee_love
        engine.runtime.energy = engine.personality.energy
        return engine

    def set_activity(self, activity: str) -> None:
        self.activity = activity
        self.runtime.idle_minutes = 0
        self.pick_behavior(force=True)

    def set_emotion(self, emotion: str) -> None:
        self.emotion = emotion or "neutral"
        self.pick_behavior(force=True)

    def set_life_mode(self, mode: str) -> None:
        self.life_mode = mode or "work"

    def _activity_behaviors(self) -> list[dict[str, Any]]:
        activities = self._tables.get("activities") or {}
        activity = activities.get(self.activity) or {}
        return list(activity.get("behaviors") or [])

    def _emotion_behaviors(self) -> list[dict[str, Any]] | None:
        emotions = self._tables.get("emotions") or {}
        emotion_table = emotions.get(self.emotion) or {}
        if not emotion_table:
            return None
        activity_table = emotion_table.get(self.activity)
        if not activity_table:
            return None
        return list(activity_table.get("behaviors") or [])

    def _eligible(self, entry: dict[str, Any]) -> bool:
        behavior_id = entry.get("id", "")
        requires = entry.get("requires") or {}
        if "min_coffee_love" in requires:
            if self.personality.coffee_love < int(requires["min_coffee_love"]):
                return False
        conditions = self._tables.get("conditions") or {}
        cond = (conditions.get(behavior_id) or {}).get("requires") or {}
        if behavior_id == "coffee" and self.personality.coffee_love >= 50:
            return True
        if "energy_lt" in cond and self.runtime.energy >= int(cond["energy_lt"]):
            return False
        if "idle_minutes_gt" in cond and self.runtime.idle_minutes <= int(cond["idle_minutes_gt"]):
            return False
        return True

    def _scaled_weight(self, entry: dict[str, Any]) -> int:
        weight = int(entry.get("weight", 1))
        behavior_id = entry.get("id", "")
        if behavior_id == "coffee":
            weight = max(1, weight * self.personality.coffee_love // 50)
        elif behavior_id in ("smile", "celebrate"):
            weight = max(1, weight * self.personality.optimism // 50)
        elif behavior_id in ("look_around", "think"):
            weight = max(1, weight * self.personality.curiosity // 50)
        elif behavior_id in ("yawn", "sleep"):
            weight = max(1, weight * self.personality.sleepiness // 50)
        elif behavior_id == "wave":
            weight = max(1, weight * self.personality.playfulness // 50)
        return max(1, weight)

    def pick_behavior(self, *, force: bool = False, exclude: str | None = None) -> str:
        table = self._emotion_behaviors() or self._activity_behaviors()
        if not table:
            return self.behavior_id

        eligible = [e for e in table if self._eligible(e)]
        if exclude and force:
            alt = [e for e in eligible if e.get("id") != exclude]
            if alt:
                eligible = alt
        if not eligible:
            eligible = table

        weights = [self._scaled_weight(e) for e in eligible]
        choice = random.choices(eligible, weights=weights, k=1)[0]
        self.behavior_id = str(choice.get("id", "breathing"))
        self.behavior_label = str(choice.get("label", self.behavior_id))

        preview_pool = [e for e in eligible if e.get("id") != self.behavior_id] or eligible
        preview_weights = [self._scaled_weight(e) for e in preview_pool]
        nxt = random.choices(preview_pool, weights=preview_weights, k=1)[0]
        self.next_behavior_id = str(nxt.get("id", self.behavior_id))
        return self.behavior_id
