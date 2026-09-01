#include "net/device_config.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include <string.h>

#if __has_include("../../secrets.h")
#include "../../secrets.h"
#else
#include "../../secrets.example.h"
#endif

#ifndef TIMEZONE
#define TIMEZONE "UTC0"
#endif

static DeviceConfig gConfig;
static bool gLoaded = false;

static void loadDefaults(DeviceConfig &cfg) {
  memset(&cfg, 0, sizeof(cfg));
  strncpy(cfg.wifiSsid, WIFI_SSID, sizeof(cfg.wifiSsid) - 1);
  strncpy(cfg.wifiPass, WIFI_PASS, sizeof(cfg.wifiPass) - 1);
  strncpy(cfg.weatherApiKey, WEATHER_API_KEY, sizeof(cfg.weatherApiKey) - 1);
  strncpy(cfg.weatherCity, WEATHER_CITY, sizeof(cfg.weatherCity) - 1);
  strncpy(cfg.timezone, TIMEZONE, sizeof(cfg.timezone) - 1);
  cfg.nightStartHour = 22;
  cfg.nightEndHour = 7;
  cfg.nightBrightness = 30;
  cfg.dayBrightness = 255;
  cfg.eyesDurationMs = 45000;
  cfg.clockDurationMs = 8000;
  cfg.weatherDurationMs = 8000;
  cfg.otaPassword[0] = '\0';
}

static void applyJsonOverrides(DeviceConfig &cfg, JsonObject doc) {
  if (const char *v = doc["wifi_ssid"] | nullptr) {
    strncpy(cfg.wifiSsid, v, sizeof(cfg.wifiSsid) - 1);
  }
  if (const char *v = doc["wifi_pass"] | nullptr) {
    strncpy(cfg.wifiPass, v, sizeof(cfg.wifiPass) - 1);
  }
  if (const char *v = doc["weather_api_key"] | nullptr) {
    strncpy(cfg.weatherApiKey, v, sizeof(cfg.weatherApiKey) - 1);
  }
  if (const char *v = doc["weather_city"] | nullptr) {
    strncpy(cfg.weatherCity, v, sizeof(cfg.weatherCity) - 1);
  }
  if (const char *v = doc["timezone"] | nullptr) {
    strncpy(cfg.timezone, v, sizeof(cfg.timezone) - 1);
  }
  if (doc["night_start_hour"].is<int>()) {
    cfg.nightStartHour = doc["night_start_hour"].as<int>();
  }
  if (doc["night_end_hour"].is<int>()) {
    cfg.nightEndHour = doc["night_end_hour"].as<int>();
  }
  if (doc["night_brightness"].is<int>()) {
    cfg.nightBrightness = doc["night_brightness"].as<int>();
  }
  if (doc["day_brightness"].is<int>()) {
    cfg.dayBrightness = doc["day_brightness"].as<int>();
  }
  if (doc["eyes_duration_ms"].is<unsigned long>()) {
    cfg.eyesDurationMs = doc["eyes_duration_ms"].as<unsigned long>();
  }
  if (doc["clock_duration_ms"].is<unsigned long>()) {
    cfg.clockDurationMs = doc["clock_duration_ms"].as<unsigned long>();
  }
  if (doc["weather_duration_ms"].is<unsigned long>()) {
    cfg.weatherDurationMs = doc["weather_duration_ms"].as<unsigned long>();
  }
  if (const char *v = doc["ota_password"] | nullptr) {
    strncpy(cfg.otaPassword, v, sizeof(cfg.otaPassword) - 1);
  }
}

static bool readJsonFile(const char *path, JsonDocument &doc) {
  File f = LittleFS.open(path, "r");
  if (!f) {
    return false;
  }
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  return !err;
}

bool deviceConfigLoad(DeviceConfig &cfg) {
  loadDefaults(cfg);
  JsonDocument wifiDoc;
  if (readJsonFile("/wifi.json", wifiDoc)) {
    applyJsonOverrides(cfg, wifiDoc.as<JsonObject>());
  }
  JsonDocument configDoc;
  if (readJsonFile("/config.json", configDoc)) {
    applyJsonOverrides(cfg, configDoc.as<JsonObject>());
  }
  gConfig = cfg;
  gLoaded = true;
  return true;
}

const DeviceConfig &deviceConfig() {
  if (!gLoaded) {
    deviceConfigLoad(gConfig);
  }
  return gConfig;
}

bool deviceConfigSaveWifi(const DeviceConfig &cfg) {
  JsonDocument doc;
  doc["wifi_ssid"] = cfg.wifiSsid;
  doc["wifi_pass"] = cfg.wifiPass;
  doc["weather_api_key"] = cfg.weatherApiKey;
  doc["weather_city"] = cfg.weatherCity;
  doc["timezone"] = cfg.timezone;
  File f = LittleFS.open("/wifi.json", "w");
  if (!f) {
    return false;
  }
  if (serializeJson(doc, f) == 0) {
    f.close();
    return false;
  }
  f.close();
  gConfig = cfg;
  gLoaded = true;
  return true;
}

bool deviceConfigSaveSettings(const DeviceConfig &cfg) {
  JsonDocument doc;
  doc["night_start_hour"] = cfg.nightStartHour;
  doc["night_end_hour"] = cfg.nightEndHour;
  doc["night_brightness"] = cfg.nightBrightness;
  doc["day_brightness"] = cfg.dayBrightness;
  doc["eyes_duration_ms"] = cfg.eyesDurationMs;
  doc["clock_duration_ms"] = cfg.clockDurationMs;
  doc["weather_duration_ms"] = cfg.weatherDurationMs;
  if (cfg.otaPassword[0]) {
    doc["ota_password"] = cfg.otaPassword;
  }
  File f = LittleFS.open("/config.json", "w");
  if (!f) {
    return false;
  }
  if (serializeJson(doc, f) == 0) {
    f.close();
    return false;
  }
  f.close();
  gConfig = cfg;
  gLoaded = true;
  return true;
}

bool deviceConfigHasWifiFile() {
  return LittleFS.exists("/wifi.json");
}
