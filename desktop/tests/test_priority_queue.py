"""Priority queue tests."""

from nomabot.protocol.commands import PlayAnimationParams, build_command
from nomabot.types import Priority
from nomabot_desktop.core.priority_queue import PriorityQueue


def test_priority_ordering() -> None:
    q = PriorityQueue()
    low = build_command("ping")
    high = build_command("play_animation", PlayAnimationParams(animation="coding"))
    q.enqueue(low, priority=Priority.LOW)
    q.enqueue(high, priority=Priority.HIGH)
    first, _ = q.dequeue()  # type: ignore[misc]
    assert first is not None
    assert first.cmd == "play_animation"
