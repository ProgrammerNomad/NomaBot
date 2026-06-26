"""Drains priority queue and sends via TransportManager."""

from __future__ import annotations

import logging

from nomabot.protocol.envelope import Envelope
from nomabot_desktop.core.priority_queue import PriorityQueue
from nomabot_desktop.transport.manager import TransportManager

logger = logging.getLogger("noma.runtime")


class CommandDispatcher:
    def __init__(self, queue: PriorityQueue, transport_manager: TransportManager) -> None:
        self._queue = queue
        self._tm = transport_manager
        self._default_device_id: str | None = None

    def set_default_device(self, device_id: str) -> None:
        self._default_device_id = device_id

    async def flush(self) -> int:
        """Send all queued commands. Returns count sent."""
        sent = 0
        while True:
            item = self._queue.dequeue()
            if item is None:
                break
            envelope, device_id = item
            did = device_id or self._default_device_id
            if not did:
                logger.warning("No device for envelope %s", envelope.cmd)
                continue
            await self._tm.send_envelope(did, envelope)
            logger.info("Dispatched %s to %s", envelope.cmd, did)
            sent += 1
        return sent

    async def dispatch_one(self, envelope: Envelope, device_id: str | None = None) -> None:
        did = device_id or self._default_device_id
        if did:
            await self._tm.send_envelope(did, envelope)
