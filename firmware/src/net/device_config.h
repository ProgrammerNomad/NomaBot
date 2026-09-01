#pragma once

#include <stddef.h>

struct DeviceConfig {
  char wifiSsid[64];
  char wifiPass[64];
  char weatherApiKey[48];
  char weatherCity[48];
  char timezone[32];
  int nightStartHour = 22;
  int nightEndHour = 7;
  int nightBrightness = 30;
  int dayBrightness = 255;
  unsigned long eyesDurationMs = 45000;
  unsigned long clockDurationMs = 8000;
  unsigned long weatherDurationMs = 8000;
  char otaPassword[32];
};

bool deviceConfigLoad(DeviceConfig &cfg);
const DeviceConfig &deviceConfig();
bool deviceConfigSaveWifi(const DeviceConfig &cfg);
bool deviceConfigSaveSettings(const DeviceConfig &cfg);
bool deviceConfigHasWifiFile();
