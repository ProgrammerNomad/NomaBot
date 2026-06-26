"""Typed command parameter models."""

from __future__ import annotations

import uuid
from typing import Any

from pydantic import BaseModel

from nomabot.protocol.envelope import Envelope, MessageType


class HelloParams(BaseModel):
    client_version: str = "0.1.0"
    protocol_max: int = 1


class PlayAnimationParams(BaseModel):
    animation: str
    loop: bool = True
    transition: str = "instant"


class ShowMessageParams(BaseModel):
    text: str
    style: str = "speech"
    duration_ms: int = 5000


class SetBackgroundParams(BaseModel):
    background: str


class SetStateParams(BaseModel):
    state: str


class PingParams(BaseModel):
    pass


class GetStatusParams(BaseModel):
    pass


COMMAND_MODELS: dict[str, type[BaseModel]] = {
    "hello": HelloParams,
    "ping": PingParams,
    "get_status": GetStatusParams,
    "play_animation": PlayAnimationParams,
    "show_message": ShowMessageParams,
    "set_background": SetBackgroundParams,
    "set_state": SetStateParams,
}


def build_command(cmd: str, params: BaseModel | dict[str, Any] | None = None) -> Envelope:
    """Build a typed command envelope."""
    if isinstance(params, BaseModel):
        param_dict = params.model_dump()
    elif params is None:
        param_dict = {}
    else:
        if cmd in COMMAND_MODELS:
            param_dict = COMMAND_MODELS[cmd].model_validate(params).model_dump()
        else:
            param_dict = params

    return Envelope(
        v=1,
        id=str(uuid.uuid4()),
        type=MessageType.COMMAND,
        cmd=cmd,
        params=param_dict,
    )


def build_response(
    request_id: str,
    cmd: str,
    *,
    ok: bool = True,
    data: dict[str, Any] | None = None,
    error: dict[str, Any] | None = None,
) -> Envelope:
    return Envelope(
        v=1,
        id=request_id,
        type=MessageType.RESPONSE if ok else MessageType.ERROR,
        cmd=cmd,
        ok=ok,
        data=data,
        error=error,
    )


def validate_command_params(cmd: str, params: dict[str, Any] | None) -> list[str]:
    """Return list of validation errors (empty if valid)."""
    if cmd not in COMMAND_MODELS:
        return []
    model = COMMAND_MODELS[cmd]
    try:
        model.model_validate(params or {})
        return []
    except Exception as e:
        return [str(e)]
