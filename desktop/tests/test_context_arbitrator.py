"""ContextArbitrator tests."""

from __future__ import annotations

import time

from nomabot.types import Priority
from nomabot_desktop.core.command_source import CommandSource
from nomabot_desktop.core.context_arbitrator import ContextArbitrator
from nomabot_desktop.core.events import StateRequest


def test_dev_panel_blocks_activity_while_override_active() -> None:
    arb = ContextArbitrator()
    assert arb.accept(
        StateRequest(state="coding", source=CommandSource.DEV_PANEL, priority=Priority.NORMAL)
    )
    blocked = arb.accept(
        StateRequest(state="idle", source=CommandSource.ACTIVITY, priority=Priority.NORMAL)
    )
    assert blocked is None
    assert arb.override_active()


def test_override_expires_after_30_seconds() -> None:
    arb = ContextArbitrator()
    arb.accept(
        StateRequest(state="coding", source=CommandSource.DEV_PANEL, priority=Priority.NORMAL)
    )
    assert arb.accept(
        StateRequest(state="idle", source=CommandSource.ACTIVITY, priority=Priority.NORMAL)
    ) is None

    arb._override.expires_at = time.monotonic() - 1  # noqa: SLF001

    assert not arb.override_active()
    assert arb.accept(
        StateRequest(state="idle", source=CommandSource.ACTIVITY, priority=Priority.NORMAL)
    ) is not None


def test_user_override_blocks_life_mode() -> None:
    arb = ContextArbitrator()
    arb.accept(
        StateRequest(
            state="idle",
            source=CommandSource.USER,
            life_mode="home",
            priority=Priority.NORMAL,
        )
    )
    blocked = arb.accept(
        StateRequest(
            state="idle",
            source=CommandSource.LIFE_MODE,
            life_mode="work",
            priority=Priority.LOW,
        )
    )
    assert blocked is None


def test_habit_from_dev_panel_not_blocked() -> None:
    arb = ContextArbitrator()
    arb.accept(
        StateRequest(state="coding", source=CommandSource.DEV_PANEL, priority=Priority.NORMAL)
    )
    habit = arb.accept(
        StateRequest(
            state="coding",
            source=CommandSource.DEV_PANEL,
            habit="morning",
            priority=Priority.NORMAL,
        )
    )
    assert habit is not None
