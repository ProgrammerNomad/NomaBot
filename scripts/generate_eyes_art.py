#!/usr/bin/env python3
"""Generate landscape eyes-only character pack art (320x170, Desper-style)."""

from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
PACK = ROOT / "assets" / "characters" / "eyes"
W, H = 320, 170
EYES_DIR = PACK / "sprites" / "eyes"
BG_DIR = PACK / "sprites" / "bg"
ANIM_DIR = PACK / "animations"

CYAN = (0, 248, 255)
BLACK = (0, 0, 0)
EYE_SIZE = 56
PUPIL_SIZE = 16
PUPIL_OFFSET_X = 8
PUPIL_OFFSET_Y = -8


def _eye_centers() -> tuple[tuple[int, int], tuple[int, int]]:
    return (96, 85), (224, 85)


def _draw_square_eye(
    draw: ImageDraw.ImageDraw,
    cx: int,
    cy: int,
    *,
    pupil_dx: int = 0,
    pupil_dy: int = 0,
    lid_close: float = 0.0,
    eye_height: int | None = None,
) -> None:
    half = EYE_SIZE // 2
    eh = eye_height if eye_height is not None else EYE_SIZE
    top = cy - eh // 2
    bottom = cy + eh // 2
    left = cx - half
    right = cx + half

    if lid_close >= 0.95:
        bar_h = max(4, eh // 8)
        draw.rectangle([left, cy - bar_h // 2, right, cy + bar_h // 2], fill=CYAN)
        return

    draw.rectangle([left, top, right, bottom], fill=CYAN)

    if lid_close > 0.05:
        cover = int(eh * lid_close)
        draw.rectangle([left, top, right, top + cover], fill=BLACK)
        return

    px = cx + PUPIL_OFFSET_X + pupil_dx - PUPIL_SIZE // 2
    py = cy + PUPIL_OFFSET_Y + pupil_dy - PUPIL_SIZE // 2
    draw.rectangle([px, py, px + PUPIL_SIZE, py + PUPIL_SIZE], fill=BLACK)


def _draw_pair(draw: ImageDraw.ImageDraw, variant: str) -> None:
    left, right = _eye_centers()
    if variant == "look_left":
        _draw_square_eye(draw, left[0], left[1], pupil_dx=-10)
        _draw_square_eye(draw, right[0], right[1], pupil_dx=-10)
    elif variant == "look_right":
        _draw_square_eye(draw, left[0], left[1], pupil_dx=10)
        _draw_square_eye(draw, right[0], right[1], pupil_dx=10)
    elif variant == "blink":
        _draw_square_eye(draw, left[0], left[1], lid_close=1.0)
        _draw_square_eye(draw, right[0], right[1], lid_close=1.0)
    elif variant == "sleepy":
        _draw_square_eye(draw, left[0], left[1], eye_height=28, lid_close=0.2)
        _draw_square_eye(draw, right[0], right[1], eye_height=28, lid_close=0.2)
    elif variant == "focus":
        _draw_square_eye(draw, left[0], left[1], eye_height=40, pupil_dy=-2)
        _draw_square_eye(draw, right[0], right[1], eye_height=40, pupil_dy=-2)
    elif variant == "happy":
        _draw_square_eye(draw, left[0], left[1], pupil_dy=-2)
        _draw_square_eye(draw, right[0], right[1], pupil_dy=-2)
    elif variant == "rain":
        _draw_square_eye(draw, left[0], left[1], eye_height=44, pupil_dy=4)
        _draw_square_eye(draw, right[0], right[1], eye_height=44, pupil_dy=4)
    else:
        _draw_square_eye(draw, left[0], left[1])
        _draw_square_eye(draw, right[0], right[1])


def eyes_sprite(variant: str, name: str) -> None:
    img = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    _draw_pair(draw, variant)
    path = EYES_DIR / f"{name}.png"
    path.parent.mkdir(parents=True, exist_ok=True)
    img.save(path)


def write_clips() -> None:
    ANIM_DIR.mkdir(parents=True, exist_ok=True)
    clips = {
        "idle": [("eyes_neutral", 400), ("eyes_neutral", 400)],
        "blink": [("eyes_blink", 120), ("eyes_neutral", 350)],
        "look_left": [("eyes_look_left", 350), ("eyes_neutral", 300)],
        "look_right": [("eyes_look_right", 350), ("eyes_neutral", 300)],
        "focus": [("eyes_focus", 280), ("eyes_focus", 280)],
        "sleepy": [("eyes_sleepy", 500), ("eyes_blink", 150)],
    }
    for clip_id, frames in clips.items():
        payload = {
            "id": clip_id,
            "loop": True,
            "frames": [{"sprite": s, "duration_ms": d} for s, d in frames],
        }
        (ANIM_DIR / f"{clip_id}.json").write_text(json.dumps(payload, indent=2), encoding="utf-8")


def main() -> None:
    BG_DIR.mkdir(parents=True, exist_ok=True)
    Image.new("RGB", (W, H), BLACK).save(BG_DIR / "dark.png")

    variants = [
        ("neutral", "eyes_neutral"),
        ("blink", "eyes_blink"),
        ("look_left", "eyes_look_left"),
        ("look_right", "eyes_look_right"),
        ("focus", "eyes_focus"),
        ("sleepy", "eyes_sleepy"),
        ("happy", "eyes_happy"),
        ("rain", "eyes_rain"),
    ]
    for variant, name in variants:
        eyes_sprite(variant, name)

    write_clips()
    print(f"Eyes pack art written under {PACK}")


if __name__ == "__main__":
    main()
