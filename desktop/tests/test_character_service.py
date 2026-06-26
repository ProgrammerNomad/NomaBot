"""CharacterService tests."""

import json
from pathlib import Path

import pytest

from nomabot_desktop.core.asset_registry import AssetRegistry
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.services.character import CharacterService
from nomabot_desktop.transport.manager import TransportManager


@pytest.fixture
def nomabot_source():
    root = Path(__file__).resolve().parents[2]
    src = root / "assets" / "characters" / "nomabot"
    if not (src / "sprites" / "body" / "body_idle_01.png").exists():
        pytest.skip("Run scripts/generate_placeholder_sprites.py first")
    return src


def test_ensure_compiled(nomabot_source, tmp_path, monkeypatch):
    monkeypatch.setattr(
        CharacterService,
        "compiled_dir",
        lambda self, pack_id: tmp_path / pack_id,
    )
    monkeypatch.setattr(
        CharacterService,
        "source_dir",
        lambda self, pack_id: nomabot_source,
    )

    assets = AssetRegistry()
    svc = CharacterService(assets, TransportManager(EventBus()))
    manifest = svc.ensure_compiled("nomabot")
    assert manifest["uuid"]
    assert manifest["version"]["major"] == 0
    assert assets.get_sprite("nomabot", "bg_office")
    report = json.loads((tmp_path / "nomabot" / "asset_report.json").read_text())
    assert report["sprites"] >= 5
