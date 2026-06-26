"""Logging setup."""

from __future__ import annotations

import logging
import os
from pathlib import Path


def setup_logging() -> None:
    appdata = os.environ.get("APPDATA", str(Path.home()))
    log_dir = Path(appdata) / "NomaBot" / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)

    fmt = logging.Formatter("%(asctime)s [%(levelname)s] %(name)s: %(message)s")

    root = logging.getLogger("noma")
    root.setLevel(logging.DEBUG)

    desktop_handler = logging.FileHandler(log_dir / "desktop.log", encoding="utf-8")
    desktop_handler.setFormatter(fmt)
    root.addHandler(desktop_handler)

    transport_handler = logging.FileHandler(log_dir / "transport.log", encoding="utf-8")
    transport_handler.setFormatter(fmt)
    logging.getLogger("noma.transport").addHandler(transport_handler)
    logging.getLogger("noma.transport").setLevel(logging.DEBUG)

    console = logging.StreamHandler()
    console.setLevel(logging.INFO)
    console.setFormatter(fmt)
    root.addHandler(console)
