#!/usr/bin/env python3
"""Copy compiled character pack into firmware LittleFS data directory."""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="Copy compiled pack to firmware/data")
    parser.add_argument(
        "--compiled",
        type=Path,
        default=Path("compiled/nomabot"),
        help="Compiled pack directory",
    )
    parser.add_argument(
        "--dest",
        type=Path,
        default=Path("firmware/data/characters/nomabot"),
        help="Firmware data destination",
    )
    parser.add_argument("--character-id", default="nomabot")
    args = parser.parse_args()

    root = Path(__file__).resolve().parents[1]
    compiled = (root / args.compiled).resolve()
    dest = (root / args.dest).resolve()
    data_root = dest.parents[1]  # firmware/data
    characters_root = data_root / "characters"

    if not compiled.exists():
        raise SystemExit(f"Compiled pack not found: {compiled}")

    if characters_root.exists():
        shutil.rmtree(characters_root)

    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(compiled, dest)

    manifest = json.loads((compiled / "manifest.json").read_text(encoding="utf-8"))
    active = {
        "character_id": args.character_id,
        "uuid": manifest.get("uuid"),
    }
    data_root.mkdir(parents=True, exist_ok=True)
    (data_root / "active_character.json").write_text(
        json.dumps(active, indent=2), encoding="utf-8"
    )
    print(f"Copied {compiled} -> {dest}")
    print(f"Wrote {data_root / 'active_character.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
