#!/usr/bin/env python3
"""Propagate root VERSION to firmware, SDK, and asset metadata."""

from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VERSION_FILE = ROOT / "VERSION"


def read_version() -> str:
    version = VERSION_FILE.read_text(encoding="utf-8").strip()
    if not re.fullmatch(r"\d+\.\d+\.\d+", version):
        raise SystemExit(f"Invalid VERSION: {version!r}")
    return version


def replace(pattern: str, repl: str, path: Path, *, count: int = 1) -> None:
    text = path.read_text(encoding="utf-8")
    new_text, n = re.subn(pattern, repl, text, count=count)
    if n == 0:
        raise SystemExit(f"Pattern not found in {path}")
    path.write_text(new_text, encoding="utf-8")


def main() -> None:
    version = read_version()
    replace(
        r'version = "[^"]+"',
        f'version = "{version}"',
        ROOT / "pyproject.toml",
    )
    replace(
        r'version = "[^"]+"',
        f'version = "{version}"',
        ROOT / "sdk" / "pyproject.toml",
    )
    replace(
        r'-D NOMA_FIRMWARE_VERSION=\\"[^"]+\\"',
        f'-D NOMA_FIRMWARE_VERSION=\\"{version}\\"',
        ROOT / "firmware" / "platformio.ini",
    )
    replace(
        r'"min_firmware": "[^"]+"',
        f'"min_firmware": "{version}"',
        ROOT / "assets" / "characters" / "eyes" / "metadata.json",
    )
    for path in (ROOT / "firmware" / "VERSION", ROOT / "assets" / "VERSION"):
        path.write_text(version + "\n", encoding="utf-8")
    print(f"Synced version {version}")


if __name__ == "__main__":
    main()
