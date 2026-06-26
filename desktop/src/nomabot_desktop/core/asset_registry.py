"""Reserved asset registry - stub for Milestone 3 character packs."""

from __future__ import annotations


class AssetRegistry:
    """Today: no-op. Tomorrow: Robot → Animation → Manifest → Loaded."""

    def __init__(self) -> None:
        self._packs: dict[str, dict] = {}

    def register_pack(self, character_id: str, manifest: dict) -> None:
        self._packs[character_id] = manifest

    def get_pack(self, character_id: str) -> dict | None:
        return self._packs.get(character_id)

    def list_packs(self) -> list[str]:
        return list(self._packs.keys())
