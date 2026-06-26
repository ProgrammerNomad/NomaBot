"""Protocol message types and serialization."""

from nomabot.protocol.commands import (
    HelloParams,
    PlayAnimationParams,
    SetBackgroundParams,
    SetStateParams,
    ShowMessageParams,
    build_command,
)
from nomabot.protocol.envelope import Envelope, MessageType, parse_line, to_json_line

__all__ = [
    "Envelope",
    "MessageType",
    "parse_line",
    "to_json_line",
    "HelloParams",
    "PlayAnimationParams",
    "ShowMessageParams",
    "SetBackgroundParams",
    "SetStateParams",
    "build_command",
]
