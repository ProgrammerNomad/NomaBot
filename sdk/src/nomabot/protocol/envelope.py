"""Protocol envelope - wire format for all messages."""

from __future__ import annotations

import json
import uuid
from enum import StrEnum
from typing import Any

from pydantic import BaseModel, Field, field_validator


class MessageType(StrEnum):
    COMMAND = "command"
    RESPONSE = "response"
    EVENT = "event"
    ERROR = "error"


class Envelope(BaseModel):
    """JSON protocol envelope - every message requires v and id."""

    v: int = 1
    id: str = Field(default_factory=lambda: str(uuid.uuid4()))
    type: MessageType
    cmd: str | None = None
    params: dict[str, Any] | None = None
    ok: bool | None = None
    data: dict[str, Any] | None = None
    error: dict[str, Any] | None = None

    @field_validator("v")
    @classmethod
    def version_must_be_positive(cls, v: int) -> int:
        if v < 1:
            raise ValueError("protocol version v must be >= 1")
        return v

    def to_dict(self) -> dict[str, Any]:
        d = self.model_dump(exclude_none=True)
        d["type"] = self.type.value
        return d

    def to_json_line(self) -> str:
        return json.dumps(self.to_dict(), separators=(",", ":")) + "\n"


def to_json_line(envelope: Envelope) -> str:
    return envelope.to_json_line()


def parse_line(line: str) -> Envelope:
    line = line.strip()
    if not line:
        raise ValueError("empty line")
    if len(line) > 16_384:
        raise ValueError("line exceeds 16KB limit")
    data = json.loads(line)
    if "v" not in data:
        raise ValueError("missing required field: v")
    if "id" not in data:
        raise ValueError("missing required field: id")
    if "type" not in data:
        raise ValueError("missing required field: type")
    return Envelope.model_validate(data)
