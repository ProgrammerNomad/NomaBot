"""BackgroundCache patch restore logic (Python mirror for tests)."""

from __future__ import annotations


class BackgroundCache:
    """Stores an RGB565 background patch for restore-before-blit."""

    def __init__(self) -> None:
        self._pixels: list[int] = []
        self._x = 0
        self._y = 0
        self._w = 0
        self._h = 0
        self._valid = False

    @property
    def valid(self) -> bool:
        return self._valid

    def reset(self) -> None:
        self._valid = False
        self._w = 0
        self._h = 0

    def capture_from_background(
        self,
        bg_pixels: list[int],
        bg_w: int,
        bg_h: int,
        patch_x: int,
        patch_y: int,
        patch_w: int,
        patch_h: int,
    ) -> bool:
        if not bg_pixels or bg_w <= 0 or bg_h <= 0 or patch_w <= 0 or patch_h <= 0:
            self.reset()
            return False

        src_x = patch_x
        src_y = patch_y
        if src_x < 0:
            patch_w += src_x
            src_x = 0
        if src_y < 0:
            patch_h += src_y
            src_y = 0
        if src_x + patch_w > bg_w:
            patch_w = bg_w - src_x
        if src_y + patch_h > bg_h:
            patch_h = bg_h - src_y
        if patch_w <= 0 or patch_h <= 0:
            self.reset()
            return False

        pixels: list[int] = []
        for row in range(patch_h):
            row_start = (src_y + row) * bg_w + src_x
            pixels.extend(bg_pixels[row_start : row_start + patch_w])

        self._pixels = pixels
        self._x = src_x
        self._y = src_y
        self._w = patch_w
        self._h = patch_h
        self._valid = True
        return True

    def restore(self, canvas: list[list[int]]) -> None:
        if not self._valid or self._w <= 0 or self._h <= 0:
            return
        idx = 0
        for row in range(self._h):
            y = self._y + row
            if y < 0 or y >= len(canvas):
                idx += self._w
                continue
            row_data = canvas[y]
            for col in range(self._w):
                x = self._x + col
                if 0 <= x < len(row_data):
                    row_data[x] = self._pixels[idx]
                idx += 1
