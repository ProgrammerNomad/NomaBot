#!/usr/bin/env python3
"""Generate placeholder sprite PNGs for nomabot character pack."""

from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
BODY = ROOT / "assets" / "characters" / "nomabot" / "sprites" / "body"


def make_sprite(name: str, color: tuple[int, int, int]) -> None:
    img = Image.new("RGB", (64, 64), (30, 30, 50))
    draw = ImageDraw.Draw(img)
    draw.ellipse([20, 8, 44, 32], fill=color)
    draw.rectangle([28, 32, 36, 56], fill=color)
    draw.line([28, 40, 12, 52], fill=color, width=3)
    draw.line([36, 40, 52, 52], fill=color, width=3)
    path = BODY / f"{name}.png"
    path.parent.mkdir(parents=True, exist_ok=True)
    img.save(path)
    print(f"Wrote {path}")


def main() -> None:
    make_sprite("body_idle_01", (67, 97, 238))
    make_sprite("body_idle_02", (76, 201, 240))
    make_sprite("body_coding_01", (247, 37, 133))
    make_sprite("body_coding_02", (181, 23, 158))
    bg = Image.new("RGB", (170, 320), (26, 26, 46))
    bg_path = ROOT / "assets" / "characters" / "nomabot" / "sprites" / "bg" / "office.png"
    bg_path.parent.mkdir(parents=True, exist_ok=True)
    bg.save(bg_path)
    print(f"Wrote {bg_path}")


if __name__ == "__main__":
    main()
