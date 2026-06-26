"""Application entry point."""

from __future__ import annotations

import argparse

from nomabot_desktop.core.lifecycle import run_app


def main() -> None:
    parser = argparse.ArgumentParser(description="NomaBot Desktop")
    parser.add_argument("--emulator", action="store_true", help="Use 170x320 emulator transport")
    parser.add_argument("--port", default=None, help="Serial COM port (e.g. COM3)")
    parser.add_argument("--dev", action="store_true", help="Show dev control buttons window")
    parser.add_argument("--no-activity", action="store_true", help="Disable activity detection")
    parser.add_argument(
        "--no-tray", action="store_true", help="Disable system tray (dev window only)"
    )
    args = parser.parse_args()
    run_app(
        emulator=args.emulator,
        port=args.port,
        dev=args.dev,
        no_activity=args.no_activity,
        no_tray=args.no_tray,
    )


if __name__ == "__main__":
    main()
