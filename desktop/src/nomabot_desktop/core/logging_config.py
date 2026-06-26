"""Logging setup with rotating file handlers."""

from __future__ import annotations

import logging
import os
from logging.handlers import RotatingFileHandler
from pathlib import Path

_LOG_MB = 10
_LOG_BACKUPS = 3


def _rotating_handler(path: Path, fmt: logging.Formatter) -> RotatingFileHandler:
    handler = RotatingFileHandler(
        path,
        maxBytes=_LOG_MB * 1024 * 1024,
        backupCount=_LOG_BACKUPS,
        encoding="utf-8",
    )
    handler.setFormatter(fmt)
    return handler


def setup_logging() -> Path:
    appdata = os.environ.get("APPDATA", str(Path.home()))
    log_dir = Path(appdata) / "NomaBot" / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)

    fmt = logging.Formatter("%(asctime)s [%(levelname)s] %(name)s: %(message)s")

    log_files = {
        "noma": log_dir / "desktop.log",
        "noma.transport": log_dir / "transport.log",
        "noma.runtime": log_dir / "runtime.log",
        "noma.activity": log_dir / "activity.log",
        "noma.scheduler": log_dir / "scheduler.log",
    }

    for logger_name, path in log_files.items():
        lg = logging.getLogger(logger_name)
        lg.setLevel(logging.DEBUG)
        lg.addHandler(_rotating_handler(path, fmt))

    root = logging.getLogger("noma")
    root.setLevel(logging.DEBUG)

    console = logging.StreamHandler()
    console.setLevel(logging.INFO)
    console.setFormatter(fmt)
    root.addHandler(console)

    return log_dir
