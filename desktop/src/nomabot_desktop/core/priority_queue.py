"""Priority queue for outbound render commands."""

from __future__ import annotations

import heapq
import itertools
import time
from dataclasses import dataclass, field

from nomabot.protocol.envelope import Envelope
from nomabot.types import Priority


@dataclass(order=True)
class _QueuedItem:
    sort_key: tuple[int, float, int] = field(init=False, repr=False)
    priority: Priority = field(compare=False)
    envelope: Envelope = field(compare=False)
    device_id: str | None = field(compare=False, default=None)
    _counter: int = field(compare=False, default=0)

    def __post_init__(self) -> None:
        # Higher priority first; FIFO within same priority
        self.sort_key = (-int(self.priority), time.monotonic(), self._counter)


class PriorityQueue:
    def __init__(self) -> None:
        self._heap: list[_QueuedItem] = []
        self._counter = itertools.count()

    def enqueue(
        self,
        envelope: Envelope,
        *,
        priority: Priority,
        device_id: str | None = None,
    ) -> None:
        item = _QueuedItem(
            priority=priority,
            envelope=envelope,
            device_id=device_id,
            _counter=next(self._counter),
        )
        heapq.heappush(self._heap, item)

    def dequeue(self) -> tuple[Envelope, str | None] | None:
        if not self._heap:
            return None
        item = heapq.heappop(self._heap)
        return item.envelope, item.device_id

    def __len__(self) -> int:
        return len(self._heap)
