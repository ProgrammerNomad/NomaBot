"""NomaBot CLI."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from nomabot.protocol.commands import COMMAND_MODELS, validate_command_params
from nomabot.protocol.envelope import parse_line


def cmd_protocol_lint(args: argparse.Namespace) -> int:
    path = Path(args.path)
    if not path.exists():
        print(f"Path not found: {path}", file=sys.stderr)
        return 1
    files = list(path.glob("*.json")) if path.is_dir() else [path]
    errors = 0
    for f in files:
        try:
            data = json.loads(f.read_text(encoding="utf-8"))
            env = parse_line(json.dumps(data, separators=(",", ":")))
            if env.type.value == "command" and env.cmd:
                param_errors = validate_command_params(env.cmd, env.params)
                if param_errors:
                    print(f"FAIL {f.name}: param validation: {param_errors}", file=sys.stderr)
                    errors += 1
                    continue
                if env.cmd not in COMMAND_MODELS and env.cmd not in ("load_character",):
                    print(f"WARN {f.name}: unknown command {env.cmd}")
            print(f"OK  {f.name}")
        except Exception as e:
            print(f"FAIL {f.name}: {e}", file=sys.stderr)
            errors += 1
    return 1 if errors else 0


def cmd_build_assets(args: argparse.Namespace) -> int:
    from nomabot.assets.compiler import compile_pack

    if not args.input or not args.output:
        print(
            "Usage: nomabot build-assets --input DIR --output DIR [--profile ID]",
            file=sys.stderr,
        )
        return 1
    profile = args.profile or "lilygo_tdisplay_s3"
    compile_pack(Path(args.input), Path(args.output), profile)
    print(f"Compiled {args.input} -> {args.output} (profile={profile})")
    return 0


def main() -> None:
    parser = argparse.ArgumentParser(prog="nomabot", description="NomaBot developer tools")
    sub = parser.add_subparsers(dest="command", required=True)

    lint_p = sub.add_parser("protocol", help="Protocol tools")
    lint_sub = lint_p.add_subparsers(dest="protocol_cmd", required=True)
    lint_cmd = lint_sub.add_parser("lint", help="Lint protocol JSON fixtures")
    lint_cmd.add_argument("path", help="File or directory of JSON fixtures")
    lint_cmd.set_defaults(func=cmd_protocol_lint)

    assets_p = sub.add_parser("build-assets", help="Compile character pack")
    assets_p.add_argument("--input", "-i", help="Source pack directory")
    assets_p.add_argument("--output", "-o", help="Output compiled directory")
    assets_p.add_argument("--profile", "-p", default="lilygo_tdisplay_s3", help="Device profile")
    assets_p.set_defaults(func=cmd_build_assets)

    args = parser.parse_args()
    raise SystemExit(args.func(args))


if __name__ == "__main__":
    main()
