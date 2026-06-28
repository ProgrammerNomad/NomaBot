"""Compile behavior.yaml + personality.yaml into behavior.json for firmware Brain."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

import yaml


def compile_behavior(source: Path) -> dict[str, Any]:
    behavior_path = source / "behavior.yaml"
    personality_path = source / "personality.yaml"
    if not behavior_path.exists():
        raise FileNotFoundError(f"behavior.yaml not found: {behavior_path}")

    data: dict[str, Any] = yaml.safe_load(behavior_path.read_text(encoding="utf-8")) or {}
    if personality_path.exists():
        personality_doc = yaml.safe_load(personality_path.read_text(encoding="utf-8")) or {}
        if "personality" in personality_doc:
            data["personality"] = personality_doc["personality"]
        if "likes" in personality_doc:
            data["likes"] = personality_doc["likes"]
    data.setdefault("render_mode", "text")
    return data


def write_behavior_json(source: Path, output_dir: Path) -> Path:
    compiled = compile_behavior(source)
    out = output_dir / "behavior.json"
    out.write_text(json.dumps(compiled, indent=2), encoding="utf-8")
    return out
