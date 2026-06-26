"""Activity profile mapping tests."""

from nomabot_desktop.services.activity import exe_to_profile


def test_exe_to_profile_coding() -> None:
    profiles = {"coding": ["Cursor.exe", "Code.exe"], "idle": []}
    assert exe_to_profile("Cursor.exe", profiles) == "coding"
    assert exe_to_profile("chrome.exe", profiles) == "idle"
