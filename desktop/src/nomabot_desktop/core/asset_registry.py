"""Asset registry — desktop facade over compiled packs."""

from __future__ import annotations

import json
from pathlib import Path


class AssetRegistry:
    """Sprite / Animation / Background lookups by id (manifest owned here)."""

    def __init__(self) -> None:
        self._packs: dict[str, dict] = {}
        self._sprite_index: dict[str, dict[str, dict]] = {}

    def register_pack(self, character_id: str, manifest: dict) -> None:
        self._packs[character_id] = manifest
        sprites: dict[str, dict] = {}
        for entry in manifest.get("sprites", []):
            sid = entry.get("id")
            if sid:
                sprites[sid] = entry
        self._sprite_index[character_id] = sprites

    def get_pack(self, character_id: str) -> dict | None:
        return self._packs.get(character_id)

    def list_packs(self) -> list[str]:
        return list(self._packs.keys())

    def get_sprite(self, character_id: str, sprite_id: str) -> dict | None:
        return self._sprite_index.get(character_id, {}).get(sprite_id)

    def get_animation_path(self, character_id: str, animation_id: str) -> Path | None:
        pack = self.compiled_dir(character_id)
        if not pack:
            return None
        path = pack / "animations" / f"{animation_id}.json"
        return path if path.exists() else None

    def get_animation(self, character_id: str, animation_id: str) -> dict | None:
        path = self.get_animation_path(character_id, animation_id)
        if not path:
            return None
        return json.loads(path.read_text(encoding="utf-8"))

    def get_background(self, character_id: str, background_key: str) -> dict | None:
        pack = self.get_pack(character_id)
        if not pack:
            return None
        config_path = self.compiled_dir(character_id) / "config.json"
        if not config_path.exists():
            return None
        config = json.loads(config_path.read_text(encoding="utf-8"))
        bg = config.get("backgrounds", {}).get(background_key)
        if not bg:
            return None
        sprite_id = bg.get("sprite")
        return self.get_sprite(character_id, sprite_id) if sprite_id else None

    def get_accessory(self, character_id: str, accessory_id: str) -> dict | None:
        return None

    def get_font(self, character_id: str, font_id: str) -> dict | None:
        return None

    def get_sound(self, character_id: str, sound_id: str) -> dict | None:
        return None

    def compiled_dir(self, character_id: str) -> Path | None:
        root = Path(__file__).resolve().parents[4]
        path = root / "compiled" / character_id
        return path if path.exists() else None

    def sprite_bin_path(self, character_id: str, sprite_id: str) -> Path | None:
        meta = self.get_sprite(character_id, sprite_id)
        pack_dir = self.compiled_dir(character_id)
        if not meta or not pack_dir:
            return None
        rel = meta.get("file")
        if not rel:
            return None
        path = pack_dir / rel
        return path if path.exists() else None
