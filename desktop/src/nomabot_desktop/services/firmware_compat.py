"""Firmware compatibility checks after hello handshake."""

from __future__ import annotations

import logging
import re

logger = logging.getLogger("noma.firmware")

_MIN_FIRMWARE = (0, 3, 0)
_REQUIRED_CAPS = ("load_character", "diagnostics")


def _parse_version(version: str) -> tuple[int, int, int]:
    parts = re.split(r"[.\-+]", version.strip())
    nums = [int(p) for p in parts[:3] if p.isdigit()]
    while len(nums) < 3:
        nums.append(0)
    return nums[0], nums[1], nums[2]


def check_firmware_compatible(hello_data: dict) -> list[str]:
    """Return human-readable issues (empty if firmware is M3-ready)."""
    issues: list[str] = []
    fw = str(hello_data.get("firmware") or hello_data.get("firmware_version") or "0.0.0")
    caps = hello_data.get("caps") or []

    if _parse_version(fw) < _MIN_FIRMWARE:
        issues.append(
            f"Firmware {fw} is older than 0.3.0. "
            "Re-flash from repo root: cd firmware && pio run -e lilygo_tdisplay_s3 -t upload"
        )

    for cap in _REQUIRED_CAPS:
        if cap not in caps:
            issues.append(
                f"Firmware missing capability '{cap}' (version {fw}). "
                "Re-flash firmware binary, then uploadfs, then RESET the board."
            )

    return issues


def log_firmware_issues(hello_data: dict) -> bool:
    """Log compatibility issues. Returns True if firmware is compatible."""
    issues = check_firmware_compatible(hello_data)
    for msg in issues:
        logger.error(msg)
    return not issues
