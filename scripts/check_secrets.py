#!/usr/bin/env python3
"""Block committing likely secrets."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BLOCKED = [
    re.compile(r"firmware/secrets\.h$"),
    re.compile(r"^\.env$"),
    re.compile(r"WEATHER_API_KEY\s+\"[0-9a-f]{16,}\""),
    re.compile(r"WIFI_PASS\s+\"[^\"]+\""),
]


def main() -> int:
    paths = [Path(p) for p in sys.argv[1:]]
    failed = False
    for path in paths:
        rel = path.as_posix()
        for pattern in BLOCKED:
            if pattern.search(rel):
                print(f"Blocked secret-like file/pattern: {rel}")
                failed = True
                break
        if path.is_file() and path.suffix in {".h", ".cpp", ".py", ".json"}:
            text = path.read_text(encoding="utf-8", errors="ignore")
            if "0bfe6e10" in text or "Shiv#" in text:
                print(f"Blocked known leaked credential pattern in {rel}")
                failed = True
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
