"""Friendship service tests."""

from pathlib import Path

from nomabot_desktop.services.friendship import FriendshipService
from nomabot_desktop.storage.service import StorageService


def test_welcome_day_one(tmp_path: Path) -> None:
    storage = StorageService(tmp_path / "test.db")
    svc = FriendshipService.__new__(FriendshipService)
    svc._storage = storage  # noqa: SLF001
    memory = svc.record_session()
    assert memory.days_together >= 1
    assert svc.welcome_message(memory) == "Hello."
    storage.close()


def test_welcome_day_100_with_name(tmp_path: Path) -> None:
    storage = StorageService(tmp_path / "test.db")
    storage.set_long_memory("first_seen_at", "1")
    storage.set_long_memory("days_together", "100")
    storage.set_long_memory("user_name", "Shiv")
    svc = FriendshipService.__new__(FriendshipService)
    svc._storage = storage  # noqa: SLF001
    from nomabot_desktop.services.friendship import FriendshipMemory

    memory = FriendshipMemory(first_seen_at=1, days_together=100, user_name="Shiv")
    assert svc.welcome_message(memory) == "Welcome back, Shiv!"
    storage.close()
