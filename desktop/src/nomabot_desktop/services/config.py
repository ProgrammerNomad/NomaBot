"""User configuration backed by SQLite settings."""

from __future__ import annotations

from nomabot_desktop.storage.service import StorageService


class ConfigService:
    def __init__(self, storage: StorageService) -> None:
        self._storage = storage

    def get_bool(self, key: str, default: bool = False) -> bool:
        val = self._storage.get_setting(key)
        if val is None:
            return default
        return val.lower() in ("1", "true", "yes")

    def set_bool(self, key: str, value: bool) -> None:
        self._storage.set_setting(key, "true" if value else "false")

    def get_str(self, key: str, default: str | None = None) -> str | None:
        return self._storage.get_setting(key, default)

    def set_str(self, key: str, value: str) -> None:
        self._storage.set_setting(key, value)

    @property
    def activity_enabled(self) -> bool:
        return self.get_bool("activity_enabled", True)

    @activity_enabled.setter
    def activity_enabled(self, value: bool) -> None:
        self.set_bool("activity_enabled", value)

    @property
    def muted(self) -> bool:
        return self.get_bool("muted", False)

    @muted.setter
    def muted(self, value: bool) -> None:
        self.set_bool("muted", value)

    @property
    def last_port(self) -> str | None:
        return self.get_str("last_port")

    @last_port.setter
    def last_port(self, value: str) -> None:
        self.set_str("last_port", value)
