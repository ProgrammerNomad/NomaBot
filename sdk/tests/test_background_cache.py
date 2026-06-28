"""BackgroundCache tests."""

from nomabot.render.background_cache import BackgroundCache


def _solid_bg(w: int, h: int, color: int) -> list[int]:
    return [color] * (w * h)


def test_capture_and_restore_patch() -> None:
    bg = _solid_bg(4, 4, 0x1111)
    for i in range(4):
        bg[4 + i] = 0x2222

    cache = BackgroundCache()
    assert cache.capture_from_background(bg, 4, 4, 1, 1, 2, 2)

    canvas = [[0] * 4 for _ in range(4)]
    cache.restore(canvas)
    assert canvas[1][1] == 0x2222
    assert canvas[2][2] == 0x1111


def test_capture_clamps_to_background_bounds() -> None:
    bg = _solid_bg(3, 3, 0xABCD)
    cache = BackgroundCache()
    assert cache.capture_from_background(bg, 3, 3, -1, -1, 5, 5)
    assert cache.valid


def test_invalid_capture_resets() -> None:
    cache = BackgroundCache()
    assert not cache.capture_from_background([], 0, 0, 0, 0, 10, 10)
    assert not cache.valid
