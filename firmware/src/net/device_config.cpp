#include "net/device_config.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <string.h>

#if __has_include("../../secrets.h")
#include "../../secrets.h"
#else
#include "../../secrets.example.h"
#endif

#ifndef TIMEZONE
#define TIMEZONE "UTC0"
#endif

static constexpr const char *kNvsNamespace = "nomabot";
static DeviceConfig gConfig;
static bool gLoaded = false;

static bool isPlaceholderSsid(const char *ssid) {
  return ssid == nullptr || ssid[0] == '\0' || strcmp(ssid, "your_wifi_ssid") == 0;
}

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

static File openConfigFile(const char *path, const char *mode) {
  File f = LittleFS.open(path, mode);
  if (f) {
    return f;
  }
  if (path[0] == '/') {
    f = LittleFS.open(path + 1, mode);
  }
  return f;
}

static bool readJsonFile(const char *path, JsonDocument &doc) {
  File f = openConfigFile(path, "r");
  if (!f) {
    Serial.printf("Config: cannot open %s\n", path);
    return false;
  }
  size_t fileSize = f.size();
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) {
    Serial.printf("Config: %s parse error (%s, %u bytes)\n", path, err.c_str(),
                  static_cast<unsigned>(fileSize));
    return false;
  }
  return true;
}

static bool loadWifiFromNvs(DeviceConfig &cfg) {
  Preferences prefs;
  if (!prefs.begin(kNvsNamespace, true)) {
    return false;
  }
  if (!prefs.isKey("wifi_ssid")) {
    prefs.end();
    return false;
  }
  String ssid = prefs.getString("wifi_ssid", "");
  String pass = prefs.getString("wifi_pass", "");
  String apiKey = prefs.getString("weather_api_key", "");
  String city = prefs.getString("weather_city", "");
  String tz = prefs.getString("timezone", "");
  prefs.end();

  if (ssid.isEmpty()) {
    return false;
  }
  strncpy(cfg.wifiSsid, ssid.c_str(), sizeof(cfg.wifiSsid) - 1);
  strncpy(cfg.wifiPass, pass.c_str(), sizeof(cfg.wifiPass) - 1);
  strncpy(cfg.weatherApiKey, apiKey.c_str(), sizeof(cfg.weatherApiKey) - 1);
  strncpy(cfg.weatherCity, city.c_str(), sizeof(cfg.weatherCity) - 1);
  if (!tz.isEmpty()) {
    strncpy(cfg.timezone, tz.c_str(), sizeof(cfg.timezone) - 1);
  }
  return true;
}

static bool saveWifiToNvs(const DeviceConfig &cfg) {
  Preferences prefs;
  if (!prefs.begin(kNvsNamespace, false)) {
    Serial.println("Config: NVS open failed");
    return false;
  }
  if (!prefs.putString("wifi_ssid", cfg.wifiSsid)) {
    prefs.end();
    Serial.println("Config: NVS wifi_ssid write failed");
    return false;
  }
  prefs.putString("wifi_pass", cfg.wifiPass);
  prefs.putString("weather_api_key", cfg.weatherApiKey);
  prefs.putString("weather_city", cfg.weatherCity);
  prefs.putString("timezone", cfg.timezone);
  prefs.end();
  return true;
}

static bool saveWifiJsonMirror(const DeviceConfig &cfg) {
  JsonDocument doc;
  doc["wifi_ssid"] = cfg.wifiSsid;
  doc["wifi_pass"] = cfg.wifiPass;
  doc["weather_api_key"] = cfg.weatherApiKey;
  doc["weather_city"] = cfg.weatherCity;
  doc["timezone"] = cfg.timezone;
  File f = openConfigFile("/wifi.json", "w");
  if (!f) {
    Serial.println("Config: wifi.json mirror save open failed");
    return false;
  }
  if (serializeJson(doc, f) == 0) {
    f.close();
    return false;
  }
  f.flush();
  f.close();
  return true;
}

static bool migrateWifiJsonToNvs(DeviceConfig &cfg) {
  JsonDocument wifiDoc;
  if (!readJsonFile("/wifi.json", wifiDoc)) {
    return false;
  }
  applyJsonOverrides(cfg, wifiDoc.as<JsonObject>());
  if (isPlaceholderSsid(cfg.wifiSsid)) {
    return false;
  }
  if (!saveWifiToNvs(cfg)) {
    return false;
  }
  Serial.printf("Config: migrated wifi.json to NVS (ssid=%s)\n", cfg.wifiSsid);
  return true;
}

bool deviceConfigLoad(DeviceConfig &cfg) {
  loadDefaults(cfg);

  if (!loadWifiFromNvs(cfg)) {
    if (migrateWifiJsonToNvs(cfg)) {
      // cfg already updated by migration
    } else {
      JsonDocument wifiDoc;
      if (readJsonFile("/wifi.json", wifiDoc)) {
        applyJsonOverrides(cfg, wifiDoc.as<JsonObject>());
      }
    }
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

void deviceConfigReload() {
  gLoaded = false;
  deviceConfigLoad(gConfig);
}

bool deviceConfigSaveWifi(const DeviceConfig &cfg) {
  if (isPlaceholderSsid(cfg.wifiSsid)) {
    Serial.println("Config: refuse to save placeholder SSID");
    return false;
  }
  if (!saveWifiToNvs(cfg)) {
    return false;
  }
  saveWifiJsonMirror(cfg);
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
  File f = openConfigFile("/config.json", "w");
  if (!f) {
    return false;
  }
  if (serializeJson(doc, f) == 0) {
    f.close();
    return false;
  }
  f.flush();
  f.close();
  gConfig = cfg;
  gLoaded = true;
  return true;
}

bool deviceConfigHasWifiFile() {
  File f = openConfigFile("/wifi.json", "r");
  if (!f) {
    return false;
  }
  f.close();
  return true;
}

bool deviceConfigHasNvsWifi() {
  Preferences prefs;
  if (!prefs.begin(kNvsNamespace, true)) {
    return false;
  }
  bool hasKey = prefs.isKey("wifi_ssid");
  String ssid = hasKey ? prefs.getString("wifi_ssid", "") : "";
  prefs.end();
  return hasKey && !ssid.isEmpty() && !isPlaceholderSsid(ssid.c_str());
}

bool deviceConfigHasValidWifi() {
  const DeviceConfig &cfg = deviceConfig();
  return !isPlaceholderSsid(cfg.wifiSsid);
}

bool deviceConfigVerifySavedWifi(const DeviceConfig &expected) {
  DeviceConfig verify;
  loadDefaults(verify);
  if (!loadWifiFromNvs(verify)) {
    Serial.println("Config: verify failed - NVS read back empty");
    return false;
  }
  if (strcmp(verify.wifiSsid, expected.wifiSsid) != 0) {
    Serial.printf("Config: verify failed - ssid mismatch (%s vs %s)\n", verify.wifiSsid,
                  expected.wifiSsid);
    return false;
  }
  return true;
}
