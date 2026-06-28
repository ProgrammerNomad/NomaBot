"""Command source precedence for world-context arbitration."""

from __future__ import annotations

from enum import Enum


class CommandSource(str, Enum):
    BRAIN = "brain"
    ROUTINE = "routine"
    LIFE_MODE = "life_mode"
    SCHEDULER = "scheduler"
    ACTIVITY = "activity"
    PLUGIN = "plugin"
    USER = "user"
    DEV_PANEL = "dev_panel"
    SYSTEM = "system"


PRECEDENCE: dict[CommandSource, int] = {
    CommandSource.BRAIN: 0,
    CommandSource.ROUTINE: 1,
    CommandSource.LIFE_MODE: 2,
    CommandSource.SCHEDULER: 3,
    CommandSource.ACTIVITY: 4,
    CommandSource.PLUGIN: 5,
    CommandSource.USER: 6,
    CommandSource.DEV_PANEL: 7,
    CommandSource.SYSTEM: 8,
}


def coerce_source(source: CommandSource | str) -> CommandSource:
    if isinstance(source, CommandSource):
        return source
    text = str(source)
    if text.startswith("routine:"):
        return CommandSource.ROUTINE
    aliases = {
        "dev": CommandSource.DEV_PANEL,
        "startup": CommandSource.SYSTEM,
        "build": CommandSource.SYSTEM,
        "season": CommandSource.SYSTEM,
        "friendship": CommandSource.USER,
    }
    if text in aliases:
        return aliases[text]
    try:
        return CommandSource(text)
    except ValueError:
        return CommandSource.BRAIN


def source_rank(source: CommandSource | str) -> int:
    return PRECEDENCE.get(coerce_source(source), 0)
