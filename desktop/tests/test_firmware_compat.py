"""Firmware compatibility checks."""

from nomabot_desktop.services.firmware_compat import check_firmware_compatible, log_firmware_issues


def test_old_firmware_missing_caps():
    hello = {
        "firmware": "0.1.0",
        "caps": ["play_animation", "show_message"],
    }
    issues = check_firmware_compatible(hello)
    assert any("0.3.0" in i for i in issues)
    assert any("load_character" in i for i in issues)
    assert not log_firmware_issues(hello)


def test_m3_firmware_ok():
    hello = {
        "firmware_version": "0.3.0",
        "caps": [
            "play_animation",
            "load_character",
            "diagnostics",
        ],
    }
    assert check_firmware_compatible(hello) == []
    assert log_firmware_issues(hello)
