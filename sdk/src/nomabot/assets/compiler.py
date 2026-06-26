"""Minimal asset compiler v0."""

from __future__ import annotations

import json
import shutil
import struct
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    Image = None  # type: ignore


def _load_profile(profile_id: str) -> dict:
    root = Path(__file__).resolve().parents[4]
    profile_path = root / "profiles" / f"{profile_id}.json"
    if not profile_path.exists():
        raise FileNotFoundError(f"Profile not found: {profile_path}")
    return json.loads(profile_path.read_text(encoding="utf-8"))


def _png_to_rgb565(path: Path) -> bytes:
    if Image is None:
        raise RuntimeError("Pillow required for asset compilation: uv add pillow --package nomabot")
    img = Image.open(path).convert("RGB")
    pixels = []
    for r, g, b in img.convert("RGB").get_flattened_data():
        rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        pixels.append(struct.pack("<H", rgb565))
    return b"".join(pixels)


def compile_pack(source: Path, output: Path, profile_id: str) -> None:
    profile = _load_profile(profile_id)
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)

    # Copy JSON metadata
    for name in ("config.json", "metadata.json", "animation_graph.json", "personality.yaml"):
        src = source / name
        if src.exists():
            shutil.copy2(src, output / name)

    sprites_out = output / "sprites"
    sprites_out.mkdir(exist_ok=True)
    manifest_sprites = []

    sprites_dir = source / "sprites"
    if sprites_dir.exists():
        for png in sorted(sprites_dir.rglob("*.png")):
            rel = png.relative_to(sprites_dir).with_suffix(".bin")
            out_bin = sprites_out / rel
            out_bin.parent.mkdir(parents=True, exist_ok=True)
            data = _png_to_rgb565(png)
            out_bin.write_bytes(data)
            if Image:
                w, h = Image.open(png).size
            else:
                w, h = 0, 0
            manifest_sprites.append(
                {
                    "id": str(rel.with_suffix("")).replace("\\", "/"),
                    "file": f"sprites/{rel.as_posix()}",
                    "width": w,
                    "height": h,
                    "bytes": len(data),
                }
            )

    animations_dir = source / "animations"
    if animations_dir.exists():
        shutil.copytree(animations_dir, output / "animations")

    manifest = {
        "pack_id": source.name,
        "version": "0.1.0",
        "profile": profile_id,
        "display": profile["display"],
        "sprites": manifest_sprites,
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
