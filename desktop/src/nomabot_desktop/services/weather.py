"""Fetch weather from OpenWeatherMap and push to device."""

from __future__ import annotations

import logging
import urllib.error
import urllib.parse
import urllib.request
import json

from PySide6.QtCore import QTimer

from nomabot.protocol.commands import SetWeatherParams, build_command
from nomabot_desktop.core.command_dispatcher import CommandDispatcher
from nomabot_desktop.services.config import ConfigService

logger = logging.getLogger("noma.weather")

_ICON_MAP = {
    "01d": "sun",
    "01n": "sun",
    "02d": "cloud",
    "02n": "cloud",
    "03d": "cloud",
    "03n": "cloud",
    "04d": "cloud",
    "04n": "cloud",
    "09d": "rain",
    "09n": "rain",
    "10d": "rain",
    "10n": "rain",
    "11d": "storm",
    "11n": "storm",
    "13d": "cloud",
    "13n": "cloud",
    "50d": "cloud",
    "50n": "cloud",
}


class WeatherService:
    def __init__(
        self,
        dispatcher: CommandDispatcher,
        config: ConfigService,
        device_id: str,
    ) -> None:
        self._dispatcher = dispatcher
        self._config = config
        self._device_id = device_id
        self._timer = QTimer()
        self._timer.timeout.connect(self._tick)

    def start(self) -> None:
        if not self._config.weather_enabled:
            logger.info("WeatherService disabled (no API key or weather_enabled=false)")
            return
        self._tick()
        self._timer.start(900_000)
        logger.info("WeatherService started")

    def stop(self) -> None:
        self._timer.stop()

    def _tick(self) -> None:
        api_key = self._config.weather_api_key
        city = self._config.weather_city
        if not api_key or not city:
            return
        q = urllib.parse.urlencode({"q": city, "appid": api_key, "units": "metric"})
        url = f"https://api.openweathermap.org/data/2.5/weather?{q}"
        try:
            with urllib.request.urlopen(url, timeout=15) as resp:
                data = json.loads(resp.read().decode("utf-8"))
        except (urllib.error.URLError, TimeoutError, json.JSONDecodeError) as exc:
            logger.warning("Weather fetch failed: %s", exc)
            return

        temp = float(data.get("main", {}).get("temp", 0))
        condition = str(data.get("weather", [{}])[0].get("main", ""))
        icon_code = str(data.get("weather", [{}])[0].get("icon", "03d"))
        icon = _ICON_MAP.get(icon_code, "cloud")
        cmd = build_command(
            "set_weather",
            SetWeatherParams(temp_c=temp, condition=condition, icon=icon, city=city),
        )
        self._dispatcher.enqueue(cmd, device_id=self._device_id)
        logger.info("Weather pushed: %.0fC %s (%s)", temp, condition, city)
