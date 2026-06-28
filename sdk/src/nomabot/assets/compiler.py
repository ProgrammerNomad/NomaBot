"""Minimal asset compiler v0 - manifest v1."""

from __future__ import annotations

import json
import re
import shutil
import struct
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    Image = None  # type: ignore

from nomabot.assets.behavior_compiler import write_behavior_json


def _load_profile(profile_id: str) -> dict:
    root = Path(__file__).resolve().parents[4]
    profile_path = root / "profiles" / f"{profile_id}.json"
    if not profile_path.exists():
        raise FileNotFoundError(f"Profile not found: {profile_path}")
    return json.loads(profile_path.read_text(encoding="utf-8"))


def _parse_version(version: str) -> dict[str, int]:
    parts = re.split(r"[.\-+]", version.strip())
    nums = [int(p) for p in parts[:3] if p.isdigit()]
    while len(nums) < 3:
        nums.append(0)
    return {"major": nums[0], "minor": nums[1], "patch": nums[2]}


def _sprite_id(rel: Path) -> str:
    """Layer-prefixed sprite id: bg/office -> bg_office, body/body_idle_01 -> body_idle_01."""
    parts = rel.parts
    stem = rel.stem
    if len(parts) >= 2 and parts[0] == "bg":
        return f"bg_{stem}"
    if len(parts) >= 2 and parts[0] == "body":
        return stem
    if len(parts) >= 2 and parts[0] == "face":
        return stem if stem.startswith("face_") else f"face_{stem}"
    return str(rel.with_suffix("")).replace("\\", "/")


def _human_bytes(n: int) -> str:
    if n < 1024:
        return f"{n}B"
    kb = n / 1024
    if kb < 1024:
        return f"{int(kb)}KB" if kb == int(kb) else f"{kb:.1f}KB"
    mb = kb / 1024
    return f"{mb:.1f}MB"


def _png_to_rgb565(path: Path) -> bytes:
    if Image is None:
        raise RuntimeError("Pillow required for asset compilation: uv add pillow --package nomabot")
    img = Image.open(path).convert("RGB")
    pixels = []
    for r, g, b in img.convert("RGB").get_flattened_data():
        rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        pixels.append(struct.pack("<H", rgb565))
    return b"".join(pixels)


def compile_pack(source: Path, output: Path, profile_id: str) -> dict:
    """Compile character pack; returns asset_report dict."""
    profile = _load_profile(profile_id)
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)

    metadata: dict = {}
    meta_path = source / "metadata.json"
    if meta_path.exists():
        metadata = json.loads(meta_path.read_text(encoding="utf-8"))
        shutil.copy2(meta_path, output / "metadata.json")

    for name in ("config.json", "animation_graph.json", "personality.yaml"):
        src = source / name
        if src.exists():
            shutil.copy2(src, output / name)

    sprites_out = output / "sprites"
    sprites_out.mkdir(exist_ok=True)
    manifest_sprites: list[dict] = []
    total_bytes = 0
    frame_count = 0

    sprites_dir = source / "sprites"
    if sprites_dir.exists():
        for png in sorted(sprites_dir.rglob("*.png")):
            rel = png.relative_to(sprites_dir)
            out_rel = rel.with_suffix(".bin")
            out_bin = sprites_out / out_rel
            out_bin.parent.mkdir(parents=True, exist_ok=True)
            data = _png_to_rgb565(png)
            out_bin.write_bytes(data)
            if Image:
                w, h = Image.open(png).size
            else:
                w, h = 0, 0
            total_bytes += len(data)
            manifest_sprites.append(
                {
                    "id": _sprite_id(rel),
                    "file": f"sprites/{out_rel.as_posix()}",
                    "width": w,
                    "height": h,
                    "bytes": len(data),
                }
            )

    animations_dir = source / "animations"
    if animations_dir.exists():
        shutil.copytree(animations_dir, output / "animations")
        for anim_file in (output / "animations").glob("*.json"):
            anim = json.loads(anim_file.read_text(encoding="utf-8"))
            frame_count += len(anim.get("frames", []))

    if (source / "behavior.yaml").exists():
        write_behavior_json(source, output)

    pack_id = metadata.get("id", source.name)
    version_str = metadata.get("version", "0.3.0")
    uuid = metadata.get("uuid", "00000000-0000-0000-0000-000000000000")
    display = profile["display"]

    manifest = {
        "pack_id": pack_id,
        "uuid": uuid,
        "version": _parse_version(version_str),
        "protocol_min": 1,
        "profile": profile_id,
        "display": {
            "profile": profile_id,
            "width": display["width"],
            "height": display["height"],
        },
        "hash": None,
        "signature": None,
        "total_bytes": total_bytes,
        "config_ref": "config.json",
        "graph_ref": "animation_graph.json",
        "animations_dir": "animations",
        "sprites": manifest_sprites,
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")

    asset_report = {
        "pack_id": pack_id,
        "uuid": uuid,
        "sprites": len(manifest_sprites),
        "frames": frame_count,
        "total_bytes": total_bytes,
        "memory_human": _human_bytes(total_bytes),
        "build": version_str,
    }
    (output / "asset_report.json").write_text(json.dumps(asset_report, indent=2), encoding="utf-8")
    return asset_report
