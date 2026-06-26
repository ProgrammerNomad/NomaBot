"""Application entry point."""

from __future__ import annotations

import argparse

from nomabot_desktop.core.lifecycle import run_app


def main() -> None:
    parser = argparse.ArgumentParser(description="NomaBot Desktop")
    parser.add_argument("--emulator", action="store_true", help="Show 170x320 emulator window")
    parser.add_argument("--port", default=None, help="Serial COM port (e.g. COM7)")
    args = parser.parse_args()
    run_app(emulator=args.emulator, port=args.port)


if __name__ == "__main__":
    main()
