"""Asset compiler tests."""

import json
from pathlib import Path

import pytest

from nomabot.assets.compiler import compile_pack


@pytest.fixture
def eyes_source() -> Path:
    root = Path(__file__).resolve().parents[2]
    src = root / "assets" / "characters" / "eyes"
    if not (src / "sprites" / "eyes" / "eyes_neutral.png").exists():
        pytest.skip("Run scripts/generate_eyes_art.py first")
    return src


def test_compile_eyes(eyes_source: Path, tmp_path: Path) -> None:
    out = tmp_path / "compiled"
    report = compile_pack(eyes_source, out, "lilygo_tdisplay_s3_landscape")
    assert (out / "manifest.json").exists()
    assert (out / "asset_report.json").exists()
    assert (out / "sprites").exists()

    manifest = json.loads((out / "manifest.json").read_text(encoding="utf-8"))
    assert manifest["pack_id"] == "eyes"
    assert manifest["display"]["width"] == 320
    assert manifest["display"]["height"] == 170

    ids = {s["id"] for s in manifest["sprites"]}
    assert "bg_dark" in ids
    assert "eyes_neutral" in ids
    assert "eyes_blink" in ids

    assert report["sprites"] == len(manifest["sprites"])
    assert report["frames"] >= 4
    assert report["memory_human"].endswith("KB")


def test_eyes_png_uses_colorkey_for_transparent_pixels(eyes_source: Path) -> None:
    import struct

    from nomabot.assets.compiler import SPRITE_COLORKEY, _png_to_rgb565

    eyes_png = eyes_source / "sprites" / "eyes" / "eyes_neutral.png"
    data = _png_to_rgb565(eyes_png, use_colorkey=True)
    assert len(data) >= 2
    assert struct.unpack_from("<H", data, 0)[0] == SPRITE_COLORKEY


def test_idle_clip_has_motion_frames(eyes_source: Path) -> None:
    idle_clip = eyes_source / "animations" / "idle.json"
    if not idle_clip.exists():
        pytest.skip("idle clip missing")
    clip = json.loads(idle_clip.read_text(encoding="utf-8"))
    sprite_ids = {frame["sprite"] for frame in clip["frames"]}
    assert "eyes_neutral" in sprite_ids
    assert len(sprite_ids) > 1
