"""Life mode service tests."""

from datetime import datetime

from nomabot_desktop.services.life_mode import life_mode_for_time


def test_night_mode_late_hours() -> None:
    assert life_mode_for_time(datetime(2026, 6, 26, 23, 0)) == "night"


def test_work_mode_weekday_morning() -> None:
    assert life_mode_for_time(datetime(2026, 6, 26, 10, 0)) == "work"


def test_home_mode_evening() -> None:
    assert life_mode_for_time(datetime(2026, 6, 26, 19, 0)) == "home"


def test_vacation_weekend() -> None:
    assert life_mode_for_time(datetime(2026, 6, 28, 12, 0)) == "vacation"
