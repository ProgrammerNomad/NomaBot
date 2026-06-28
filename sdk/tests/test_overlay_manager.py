"""OverlayManager ID and priority tests."""

from nomabot.render import OverlayManager


def test_same_id_replaces_text() -> None:
    om = OverlayManager()
    om.push("dev_hello", "Hello", 2, 5000, 1000)
    om.push("dev_hello", "Hello again", 2, 5000, 2000)
    assert om.active_text() == "Hello again"
    assert om.queue_depth() == 1


def test_lower_priority_rejected() -> None:
    om = OverlayManager()
    om.push("build_failed", "Failed", 3, 5000, 1000)
    om.push("dev_hello", "Hello", 2, 5000, 1000)
    assert om.active_text() == "Failed"


def test_cancel_by_id() -> None:
    om = OverlayManager()
    om.push("dev_hello", "Hello", 2, 5000, 1000)
    assert om.cancel("dev_hello")
    assert om.active_text() == ""
    assert om.queue_depth() == 0


def test_tick_expires() -> None:
    om = OverlayManager()
    om.push("dev_hello", "Hello", 2, 1000, 1000)
    assert om.tick(2500)
    assert om.active_text() == ""
