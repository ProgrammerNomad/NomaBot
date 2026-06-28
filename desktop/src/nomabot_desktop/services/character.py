"""Character pack compile + device activation."""

from __future__ import annotations

import json
import logging
import subprocess
import sys
from pathlib import Path

from nomabot.assets.compiler import compile_pack
from nomabot.client import NomaClient
from nomabot.protocol.commands import LoadCharacterParams, build_command
from nomabot_desktop.core.asset_registry import AssetRegistry
from nomabot_desktop.transport.manager import TransportManager

logger = logging.getLogger("noma.character")


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


class CharacterService:
    """Owns manifest knowledge; desktop Runtime must not parse manifest.json directly."""

    def __init__(self, assets: AssetRegistry, transport: TransportManager) -> None:
        self._assets = assets
        self._transport = transport
        self._active_pack_id: str | None = None
        self._active_uuid: str | None = None

    @property
    def active_pack_id(self) -> str | None:
        return self._active_pack_id

    @property
    def active_uuid(self) -> str | None:
        return self._active_uuid

    def source_dir(self, pack_id: str) -> Path:
        return _repo_root() / "assets" / "characters" / pack_id

    def compiled_dir(self, pack_id: str) -> Path:
        return _repo_root() / "compiled" / pack_id

    def ensure_compiled(self, pack_id: str, *, profile: str = "lilygo_tdisplay_s3") -> dict:
        source = self.source_dir(pack_id)
        output = self.compiled_dir(pack_id)
        manifest_path = output / "manifest.json"

        needs_build = True
        if manifest_path.exists() and (source / "metadata.json").exists():
            src_meta = json.loads((source / "metadata.json").read_text(encoding="utf-8"))
            out_manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
            if out_manifest.get("uuid") == src_meta.get("uuid"):
                needs_build = False

        if needs_build:
            logger.info("Compiling character pack %s", pack_id)
            compile_pack(source, output, profile)
            self._copy_to_firmware_data(pack_id, output)

        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        self._assets.register_pack(pack_id, manifest)
        return manifest

    def _copy_to_firmware_data(self, pack_id: str, compiled: Path) -> None:
        script = _repo_root() / "scripts" / "copy_pack_to_firmware_data.py"
        if not script.exists():
            return
        subprocess.run(
            [
                sys.executable,
                str(script),
                "--compiled",
                str(compiled.relative_to(_repo_root())),
                "--character-id",
                pack_id,
            ],
            cwd=_repo_root(),
            check=False,
        )

    async def activate(self, device_id: str, pack_id: str) -> dict | None:
        manifest = self.ensure_compiled(pack_id)
        adapter = self._transport._adapters.get(device_id)  # noqa: SLF001
        if not adapter:
            logger.warning("No transport for %s", device_id)
            return None

        client = NomaClient(adapter._inner)  # noqa: SLF001
        resp = await client.send_command(
            build_command("load_character", LoadCharacterParams(character_id=pack_id))
        )
        if not resp.ok:
            detail = resp.error or (resp.data or {}).get("error", "unknown")
            logger.error("load_character failed: %s", detail)
            return None

        data = resp.data or {}
        self._active_pack_id = pack_id
        self._active_uuid = data.get("uuid") or manifest.get("uuid")
        logger.info("Activated character %s uuid=%s", pack_id, self._active_uuid)
        return data
