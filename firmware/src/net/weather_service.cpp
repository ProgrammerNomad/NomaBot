#include "net/weather_service.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "character/character_runtime.h"
#include "net/wifi_service.h"

#if __has_include("../../secrets.h")
#include "../../secrets.h"
#else
#include "../../secrets.example.h"
#endif

extern WifiService gWifiService;

static const char *mapWeatherIcon(const char *iconCode) {
  if (!iconCode || !iconCode[0]) {
    return "cloud";
  }
  if (iconCode[0] == '0' && iconCode[1] == '1') {
    return "sun";
  }
  if (iconCode[0] == '1' && iconCode[1] == '1') {
    return "storm";
  }
  if (iconCode[0] == '0' && iconCode[1] == '9') {
    return "rain";
  }
  if (iconCode[0] == '1' && iconCode[1] == '0') {
    return "rain";
  }
  return "cloud";
}

void WeatherService::begin(CharacterRuntime *runtime) {
  _runtime = runtime;
  _lastFetchMs = 0;
}

bool WeatherService::fetchWeather() {
  if (!gWifiService.connected() || !_runtime) {
    return false;
  }
  if (strcmp(WEATHER_API_KEY, "your_openweathermap_key") == 0) {
    return false;
  }

  String url = "http://api.openweathermap.org/data/2.5/weather?q=";
  url += WEATHER_CITY;
  url += "&appid=";
  url += WEATHER_API_KEY;
  url += "&units=metric";

  HTTPClient http;
  http.setTimeout(15000);
  if (!http.begin(url)) {
    return false;
  }
  int code = http.GET();
  if (code != 200) {
    Serial.printf("Weather HTTP %d\n", code);
    http.end();
    return false;
  }
  String payload = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, payload)) {
    return false;
  }

  float temp = doc["main"]["temp"] | 0.0f;
  const char *condition = doc["weather"][0]["main"] | "Clear";
  const char *iconCode = doc["weather"][0]["icon"] | "03d";
  const char *icon = mapWeatherIcon(iconCode);

  char line1[32];
  snprintf(line1, sizeof(line1), "%.0fC", temp);
  char line2[48];
  snprintf(line2, sizeof(line2), "%s", condition);

  _runtime->setWeatherDisplay(icon, line1, line2, WEATHER_CITY);
  Serial.printf("Weather: %s %s %s\n", line1, line2, WEATHER_CITY);
  return true;
}

void WeatherService::tick() {
  if (!gWifiService.connected()) {
    return;
  }
  unsigned long now = millis();
  if (_lastFetchMs != 0 && now - _lastFetchMs < 900000UL) {
    return;
  }
  _lastFetchMs = now;
  fetchWeather();
}
