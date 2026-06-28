#!/usr/bin/env python3
"""Generate NomaBot art at 170x320 — Prototype Pack v0 and full character pack."""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
PACK = ROOT / "assets" / "characters" / "nomabot"
PROTOTYPE = PACK / "prototype_v0"
CONCEPT = PACK / "concept"
BODY_DIR = PACK / "sprites" / "body"
FACE_DIR = PACK / "sprites" / "face"
BG_DIR = PACK / "sprites" / "bg"
ANIM_DIR = PACK / "animations"

W, H = 170, 320
HOME_X, HOME_Y = 85, 80

C = {
    "helmet": (249, 115, 22),
    "helmet_d": (194, 65, 12),
    "body": (248, 250, 252),
    "body_d": (203, 213, 225),
    "visor": (15, 23, 42),
    "eye": (56, 189, 248),
    "eye_hi": (224, 242, 254),
    "outline": (30, 41, 59),
    "desk": (146, 64, 14),
    "desk_l": (180, 83, 9),
    "wall": (30, 41, 59),
    "wall2": (51, 65, 85),
    "floor": (88, 28, 135),
    "sky": (125, 211, 252),
    "frame": (100, 116, 139),
    "plant": (34, 197, 94),
    "pot": (161, 98, 7),
    "laptop": (71, 85, 105),
    "screen": (52, 211, 153),
    "coffee": (120, 53, 15),
    "gold": (251, 191, 36),
    "mug": (249, 115, 22),
}


def _draw_apartment_minimal(draw: ImageDraw.ImageDraw) -> None:
    """Phase A placeholder — flat wall, no props."""
    draw.rectangle([0, 0, W, H], fill=C["wall2"])
    draw.rectangle([0, 200, W, H], fill=C["floor"])


def _draw_apartment_v1(draw: ImageDraw.ImageDraw) -> None:
    """Phase D — six props: window, desk, plant, laptop, coffee, shelf."""
    draw.rectangle([0, 0, W, H], fill=C["wall2"])
    draw.rectangle([0, 0, W, 48], fill=C["wall"])
    # Window
    draw.rectangle([20, 10, 88, 58], fill=C["frame"])
    draw.rectangle([24, 14, 84, 54], fill=C["sky"])
    draw.line([54, 14, 54, 54], fill=C["frame"], width=2)
    draw.line([24, 34, 84, 34], fill=C["frame"], width=2)
    # Shelf
    draw.rectangle([112, 18, 158, 26], fill=C["desk"])
    draw.rectangle([118, 10, 126, 18], fill=C["gold"])
    # Desk surface
    draw.rectangle([8, 198, 162, 210], fill=C["desk"])
    draw.rectangle([8, 210, 162, H], fill=C["desk_l"])
    # Laptop at home desk
    draw.rectangle([58, 176, 112, 198], fill=C["laptop"])
    draw.rectangle([62, 180, 108, 194], fill=C["screen"])
    # Plant
    draw.rectangle([16, 188, 30, 200], fill=C["pot"])
    draw.ellipse([12, 168, 34, 192], fill=C["plant"])
    # Coffee
    draw.rectangle([118, 184, 132, 198], fill=C["mug"])
    draw.rectangle([120, 180, 130, 184], fill=C["coffee"])


def _draw_helmet(draw: ImageDraw.ImageDraw, ox: int, oy: int, *, tilt: int = 0) -> None:
    """Helmet with optional head tilt (pixels right = thinking)."""
    hx = ox + tilt
    draw.ellipse([hx - 20, oy - 18, hx + 20, oy + 14], fill=C["helmet"], outline=C["outline"], width=2)
    draw.rectangle([hx - 16, oy + 2, hx + 16, oy + 10], fill=C["visor"])
    draw.ellipse([hx - 6, oy - 8, hx + 6, oy + 4], fill=C["gold"], outline=C["outline"], width=1)


def _draw_robot_body(draw: ImageDraw.ImageDraw, ox: int, oy: int, pose: str) -> None:
    """Draw NomaBot at head anchor (ox, oy). Silhouette-first poses for 170x320 LCD."""
    tilt = 0
    if pose == "think":
        tilt = 8
    elif pose == "blink":
        tilt = 0

    draw.rectangle([ox - 14, oy + 36, ox + 14, oy + 56], fill=C["body"], outline=C["outline"], width=2)
    draw.rounded_rectangle(
        [ox - 18, oy + 8, ox + 18, oy + 40], radius=6, fill=C["body"], outline=C["outline"], width=2
    )
    _draw_helmet(draw, ox, oy, tilt=tilt)

    if pose in ("typing", "coding"):
        draw.line([ox - 18, oy + 22, ox - 34, oy + 38], fill=C["body_d"], width=5)
        draw.line([ox + 18, oy + 22, ox + 34, oy + 38], fill=C["body_d"], width=5)
        draw.line([ox - 34, oy + 38, ox - 28, oy + 42], fill=C["outline"], width=2)
        draw.line([ox + 34, oy + 38, ox + 28, oy + 42], fill=C["outline"], width=2)
    elif pose == "think":
        draw.line([ox - 18, oy + 22, ox - 28, oy + 36], fill=C["body_d"], width=4)
        draw.line([ox + 18, oy + 18, ox + 32, oy + 6], fill=C["body_d"], width=5)
    elif pose == "coffee":
        draw.line([ox + 18, oy + 20, ox + 32, oy + 28], fill=C["body_d"], width=4)
        draw.rectangle([ox + 30, oy + 22, ox + 42, oy + 34], fill=C["mug"], outline=C["outline"], width=1)
    elif pose == "wave":
        draw.line([ox - 18, oy + 22, ox - 26, oy + 36], fill=C["body_d"], width=4)
        draw.line([ox + 18, oy + 10, ox + 32, oy - 2], fill=C["body_d"], width=5)
    elif pose == "look_left":
        draw.line([ox - 18, oy + 22, ox - 28, oy + 38], fill=C["body_d"], width=4)
        draw.line([ox + 18, oy + 22, ox + 26, oy + 36], fill=C["body_d"], width=4)
        draw.polygon([ox - 18, oy + 4, ox - 6, oy + 8, ox - 18, oy + 12], fill=C["visor"])
    elif pose == "look_right":
        draw.line([ox - 18, oy + 22, ox - 26, oy + 36], fill=C["body_d"], width=4)
        draw.line([ox + 18, oy + 22, ox + 28, oy + 38], fill=C["body_d"], width=4)
        draw.polygon([ox + 18, oy + 4, ox + 6, oy + 8, ox + 18, oy + 12], fill=C["visor"])
    elif pose == "sleep":
        draw.line([ox - 18, oy + 22, ox - 10, oy + 36], fill=C["body_d"], width=4)
        draw.line([ox + 18, oy + 22, ox + 10, oy + 36], fill=C["body_d"], width=4)
    elif pose == "celebrate":
        draw.line([ox - 18, oy + 8, ox - 30, oy - 4], fill=C["body_d"], width=5)
        draw.line([ox + 18, oy + 8, ox + 30, oy - 4], fill=C["body_d"], width=5)
    elif pose == "stretch":
        draw.line([ox - 18, oy + 14, ox - 34, oy + 0], fill=C["body_d"], width=5)
        draw.line([ox + 18, oy + 14, ox + 34, oy + 0], fill=C["body_d"], width=5)
    else:
        draw.line([ox - 18, oy + 22, ox - 26, oy + 38], fill=C["body_d"], width=4)
        draw.line([ox + 18, oy + 22, ox + 26, oy + 36], fill=C["body_d"], width=4)

    if pose == "blink":
        hx = ox + tilt
        draw.rectangle([hx - 14, oy + 2, hx + 14, oy + 10], fill=C["visor"])


def body_sprite(pose: str, name: str) -> None:
    img = Image.new("RGBA", (56, 72), (0, 0, 0, 0))
    _draw_robot_body(ImageDraw.Draw(img), 28, 28, pose)
    path = BODY_DIR / f"{name}.png"
    path.parent.mkdir(parents=True, exist_ok=True)
    img.save(path)


def _draw_face(draw: ImageDraw.ImageDraw, expression: str) -> None:
    cx, cy = 14, 12
    draw.ellipse([2, 2, 26, 22], fill=C["visor"])
    if expression in ("sleepy", "yawning"):
        draw.line([6, cy + 2, 22, cy + 2], fill=C["eye"], width=2)
    elif expression == "blink":
        draw.line([5, cy, 23, cy], fill=C["eye"], width=3)
    else:
        draw.ellipse([5, cy - 3, 11, cy + 3], fill=C["eye"])
        draw.point([7, cy - 2], fill=C["eye_hi"])
        draw.ellipse([17, cy - 3, 23, cy + 3], fill=C["eye"])
        draw.point([19, cy - 2], fill=C["eye_hi"])
    mouth_y = cy + 8
    if expression in ("happy", "smile", "proud"):
        draw.arc([7, mouth_y - 2, 21, mouth_y + 8], 0, 180, fill=C["eye"], width=2)
    elif expression in ("angry", "frustrated"):
        draw.line([8, mouth_y + 4, 20, mouth_y + 4], fill=C["eye"], width=2)
    elif expression in ("surprised", "surprise"):
        draw.ellipse([11, mouth_y, 17, mouth_y + 8], outline=C["eye"], width=2)
    elif expression in ("thinking", "confused"):
        draw.line([9, mouth_y + 4, 19, mouth_y + 2], fill=C["eye"], width=2)
    else:
        draw.line([9, mouth_y + 4, 19, mouth_y + 4], fill=C["eye"], width=2)


def face_sprite(expression: str) -> None:
    img = Image.new("RGBA", (28, 24), (0, 0, 0, 0))
    _draw_face(ImageDraw.Draw(img), expression)
    path = FACE_DIR / f"face_{expression}.png"
    path.parent.mkdir(parents=True, exist_ok=True)
    img.save(path)


def write_clips(clips: dict[str, list[str]]) -> None:
    ANIM_DIR.mkdir(parents=True, exist_ok=True)
    for clip_id, sprites in clips.items():
        frames = [{"sprite": s, "duration_ms": 450} for s in sprites]
        (ANIM_DIR / f"{clip_id}.json").write_text(
            json.dumps({"id": clip_id, "loop": True, "frames": frames}, indent=2),
            encoding="utf-8",
        )


def draw_bg(minimal: bool) -> Image.Image:
    img = Image.new("RGB", (W, H), C["wall2"])
    draw = ImageDraw.Draw(img)
    if minimal:
        _draw_apartment_minimal(draw)
    else:
        _draw_apartment_v1(draw)
    return img


def copy_prototype_refs() -> None:
    PROTOTYPE.mkdir(parents=True, exist_ok=True)
    refs = [
        ("body_stand.png", BODY_DIR / "body_stand.png"),
        ("body_typing_01.png", BODY_DIR / "body_typing_01.png"),
        ("body_think.png", BODY_DIR / "body_think.png"),
        ("body_blink_01.png", BODY_DIR / "body_blink_01.png"),
        ("face_neutral.png", FACE_DIR / "face_neutral.png"),
        ("face_happy.png", FACE_DIR / "face_happy.png"),
        ("face_thinking.png", FACE_DIR / "face_thinking.png"),
    ]
    for dest_name, src in refs:
        if src.exists():
            shutil.copy2(src, PROTOTYPE / dest_name)


def generate_prototype_v0() -> None:
    """Prototype Pack v0 — seven assets, four clips, minimal bg."""
    BODY_DIR.mkdir(parents=True, exist_ok=True)
    FACE_DIR.mkdir(parents=True, exist_ok=True)
    BG_DIR.mkdir(parents=True, exist_ok=True)

    draw_bg(minimal=True).save(BG_DIR / "office.png")

    for pose, name in [
        ("standing", "body_stand"),
        ("standing", "body_idle_01"),
        ("standing", "body_idle_02"),
        ("typing", "body_typing_01"),
        ("typing", "body_typing_02"),
        ("think", "body_think"),
        ("blink", "body_blink_01"),
    ]:
        body_sprite(pose, name)

    for expr in ("neutral", "happy", "thinking", "blink"):
        face_sprite(expr)

    write_clips(
        {
            "idle": ["body_idle_01", "body_idle_02"],
            "blink": ["body_blink_01", "body_stand"],
            "coding": ["body_typing_01", "body_typing_02", "body_typing_01"],
            "think": ["body_think", "body_think"],
        }
    )
    copy_prototype_refs()
    hero = draw_bg(minimal=True)
    _draw_robot_body(ImageDraw.Draw(hero), HOME_X, HOME_Y, "standing")
    hero.save(PROTOTYPE / "hero_home.png")


def generate_full() -> None:
    """Phase D+E — apartment v1, expanded poses, six core expressions + extras."""
    BODY_DIR.mkdir(parents=True, exist_ok=True)
    FACE_DIR.mkdir(parents=True, exist_ok=True)
    BG_DIR.mkdir(parents=True, exist_ok=True)

    draw_bg(minimal=False).save(BG_DIR / "office.png")

    poses = [
        ("standing", "body_stand"),
        ("standing", "body_idle_01"),
        ("standing", "body_idle_02"),
        ("typing", "body_typing_01"),
        ("typing", "body_typing_02"),
        ("think", "body_think"),
        ("blink", "body_blink_01"),
        ("coffee", "body_coffee"),
        ("stretch", "body_stretch"),
        ("wave", "body_wave"),
        ("celebrate", "body_celebrate"),
        ("sleep", "body_sleep"),
        ("look_left", "body_look_left"),
        ("look_right", "body_look_right"),
    ]
    for pose, name in poses:
        body_sprite(pose, name)

    for expr in (
        "neutral",
        "happy",
        "thinking",
        "sleepy",
        "surprised",
        "angry",
        "blink",
        "smile",
        "confused",
    ):
        face_sprite(expr)

    write_clips(
        {
            "idle": ["body_idle_01", "body_idle_02"],
            "blink": ["body_blink_01", "body_stand"],
            "look_left": ["body_look_left"],
            "look_right": ["body_look_right"],
            "stretch": ["body_stretch", "body_stand"],
            "coding": ["body_typing_01", "body_typing_02", "body_typing_01"],
            "think": ["body_think"],
            "coffee": ["body_coffee"],
            "celebrate": ["body_celebrate"],
            "sleep": ["body_sleep"],
            "snore": ["body_sleep", "body_idle_01"],
            "yawn": ["body_stretch", "body_sleep"],
            "wave": ["body_wave"],
            "hello": ["body_wave", "body_stand"],
            "thumbs_up": ["body_celebrate"],
        }
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate NomaBot art")
    parser.add_argument(
        "--full",
        action="store_true",
        help="Full pack: apartment v1 + expanded poses/clips (Phase D+E)",
    )
    args = parser.parse_args()
    if args.full:
        generate_full()
        print(f"Full pack written under {PACK}")
    else:
        generate_prototype_v0()
        print(f"Prototype Pack v0 written under {PACK}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
