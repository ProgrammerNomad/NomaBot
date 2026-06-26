"""Activity service - foreground window detection (Windows)."""

from __future__ import annotations

import json
import logging
import os
import sys
from pathlib import Path

from PySide6.QtCore import QTimer

from nomabot.types import Priority
from nomabot_desktop.core.bus import EventBus
from nomabot_desktop.core.events import ActivityChanged, StateRequest
from nomabot_desktop.services.config import ConfigService

logger = logging.getLogger("noma.activity")


def _load_profiles() -> dict[str, list[str]]:
    appdata = Path(os.environ.get("APPDATA", str(Path.home()))) / "NomaBot"
    user_path = appdata / "activity_profiles.json"
    repo_root = Path(__file__).resolve().parents[4]
    default_path = repo_root / "profiles" / "activity_profiles.json"
    path = user_path if user_path.exists() else default_path
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return {"coding": ["Code.exe", "Cursor.exe"], "idle": []}


def exe_to_profile(exe_name: str, profiles: dict[str, list[str]]) -> str:
    exe_lower = exe_name.lower()
    for profile, exes in profiles.items():
        if profile == "idle":
            continue
        for pattern in exes:
            if pattern.lower() == exe_lower:
                return profile
    return "idle"


def _foreground_exe_windows() -> str:
    if sys.platform != "win32":
        return ""
    import ctypes

    user32 = ctypes.windll.user32
    hwnd = user32.GetForegroundWindow()
    if not hwnd:
        return ""
    length = user32.GetWindowTextLengthW(hwnd)
    buf = ctypes.create_unicode_buffer(length + 1)
    user32.GetWindowTextW(hwnd, buf, length + 1)
    title = buf.value
    # Best-effort: try process name via GetWindowThreadProcessId
    pid = ctypes.c_ulong()
    user32.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))
    try:
        import ctypes.wintypes

        PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
        kernel32 = ctypes.windll.kernel32
        handle = kernel32.OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, pid.value)
        if handle:
            exe_buf = ctypes.create_unicode_buffer(260)
            size = ctypes.c_ulong(260)
            if hasattr(kernel32, "QueryFullProcessImageNameW"):
                kernel32.QueryFullProcessImageNameW(handle, 0, exe_buf, ctypes.byref(size))
                kernel32.CloseHandle(handle)
                return Path(exe_buf.value).name
            kernel32.CloseHandle(handle)
    except Exception:
        pass
    if "cursor" in title.lower() or "code" in title.lower():
        return "Cursor.exe"
    return ""


class ActivityService:
    def __init__(self, bus: EventBus, config: ConfigService) -> None:
        self._bus = bus
        self._config = config
        self._profiles = _load_profiles()
        self._current = "idle"
        self._stable_count = 0
        self._timer = QTimer()
        self._timer.timeout.connect(self._poll)

    def start(self) -> None:
        if not self._config.activity_enabled:
            logger.info("Activity service disabled")
            return
        self._timer.start(2000)
        logger.info("Activity service started")

    def stop(self) -> None:
        self._timer.stop()

    def _poll(self) -> None:
        if not self._config.activity_enabled:
            return
        exe = _foreground_exe_windows()
        profile = exe_to_profile(exe, self._profiles) if exe else "idle"
        if profile == self._current:
            return
        self._stable_count += 1
        if self._stable_count < 2:
            return
        self._stable_count = 0
        self._current = profile
        logger.info("Activity profile -> %s (%s)", profile, exe or "unknown")
        self._bus.publish(
            "activity.changed",
            ActivityChanged(profile=profile, exe_name=exe, priority=Priority.NORMAL),
        )
        self._bus.publish(
            "state.request",
            StateRequest(
                state=profile,
                priority=Priority.NORMAL,
                source="activity",
                animation=profile if profile != "idle" else "idle",
            ),
        )
