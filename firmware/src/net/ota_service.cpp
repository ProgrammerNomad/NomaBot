#include "net/ota_service.h"

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include "net/device_config.h"
#include "net/wifi_service.h"

extern WifiService gWifiService;

void OtaService::begin() {
  if (!gWifiService.connected()) {
    return;
  }
  const DeviceConfig &cfg = deviceConfig();
  ArduinoOTA.setHostname("eyes-ambient");
  if (cfg.otaPassword[0]) {
    ArduinoOTA.setPassword(cfg.otaPassword);
  }
  ArduinoOTA.onStart([]() { Serial.println("OTA start"); });
  ArduinoOTA.onEnd([]() { Serial.println("OTA end"); });
  ArduinoOTA.onError([](ota_error_t err) { Serial.printf("OTA error %u\n", err); });
  ArduinoOTA.begin();
  if (MDNS.begin("eyes-ambient")) {
    Serial.println("mDNS: eyes-ambient.local");
  }
}

void OtaService::tick() {
  if (gWifiService.connected()) {
    ArduinoOTA.handle();
  }
}
