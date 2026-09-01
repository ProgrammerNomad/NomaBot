"""Weather and clock command validation."""

from nomabot.protocol.commands import SetClockParams, SetWeatherParams, validate_command_params


def test_set_weather_params_valid() -> None:
    assert (
        validate_command_params(
            "set_weather",
            {"temp_c": 28.5, "condition": "Clear", "icon": "sun", "city": "Mumbai,IN"},
        )
        == []
    )


def test_set_clock_params_valid() -> None:
    assert validate_command_params("set_clock", {"time": "22:41", "date": "Mon 1 Sep"}) == []


def test_set_weather_model_defaults() -> None:
    p = SetWeatherParams(temp_c=20.0)
    assert p.icon == "cloud"
    assert p.city == ""


def test_set_clock_model() -> None:
    p = SetClockParams(time="09:05")
    assert p.date == ""
