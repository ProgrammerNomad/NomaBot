"""Asset compiler tests."""

import json
from pathlib import Path

import pytest

from nomabot.assets.compiler import compile_pack


@pytest.fixture
def nomabot_source(tmp_path):
    root = Path(__file__).resolve().parents[2]
    src = root / "assets" / "characters" / "nomabot"
    sprites = src / "sprites" / "body" / "body_idle_01.png"
    if not sprites.exists():
        pytest.skip("Run scripts/generate_placeholder_sprites.py first")
    return src


def test_compile_nomabot(nomabot_source, tmp_path):
    out = tmp_path / "compiled"
    report = compile_pack(nomabot_source, out, "lilygo_tdisplay_s3")
    assert (out / "manifest.json").exists()
    assert (out / "asset_report.json").exists()
    assert (out / "sprites").exists()

    manifest = json.loads((out / "manifest.json").read_text(encoding="utf-8"))
    assert manifest["pack_id"] == "nomabot"
    assert manifest["uuid"] == "a6d1e8f0-4c2b-4f91-9c3d-8e1a2b4c6d8e"
    assert manifest["version"] == {"major": 0, "minor": 3, "patch": 0}
    assert manifest["protocol_min"] == 1
    assert manifest["hash"] is None
    assert manifest["signature"] is None
    assert manifest["display"]["width"] == 170
    assert manifest["config_ref"] == "config.json"
    assert manifest["graph_ref"] == "animation_graph.json"

    ids = {s["id"] for s in manifest["sprites"]}
    assert "bg_office" in ids
    assert "body_idle_01" in ids
    assert "body_coding_01" in ids
    assert "body/idle_01" not in ids

    assert report["sprites"] == len(manifest["sprites"])
    assert report["frames"] >= 15
    assert report["memory_human"].endswith("KB")
