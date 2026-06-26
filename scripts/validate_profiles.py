#!/usr/bin/env python3
"""Validate device profile JSON files against profiles/schema.json."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PROFILES_DIR = ROOT / "profiles"
SCHEMA_PATH = PROFILES_DIR / "schema.json"


def main() -> int:
    try:
        import jsonschema
    except ImportError:
        print("jsonschema required: uv sync", file=sys.stderr)
        return 1

    schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
    errors = 0
    for path in sorted(PROFILES_DIR.glob("*.json")):
        if path.name == "schema.json":
            continue
        data = json.loads(path.read_text(encoding="utf-8"))
        try:
            jsonschema.validate(data, schema)
            print(f"OK  {path.name}")
        except jsonschema.ValidationError as e:
            print(f"FAIL {path.name}: {e.message}", file=sys.stderr)
            errors += 1
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
