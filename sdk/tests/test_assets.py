"""Asset compiler tests."""

from pathlib import Path

import pytest

from nomabot.assets.compiler import compile_pack


@pytest.fixture
def nomabot_source(tmp_path):
    root = Path(__file__).resolve().parents[2]
    src = root / "assets" / "characters" / "nomabot"
    if not (src / "sprites").exists():
        pytest.skip("Run scripts/generate_placeholder_sprites.py first")
    return src


def test_compile_nomabot(nomabot_source, tmp_path):
    out = tmp_path / "compiled"
    compile_pack(nomabot_source, out, "lilygo_tdisplay_s3")
    assert (out / "manifest.json").exists()
    assert (out / "sprites").exists()
