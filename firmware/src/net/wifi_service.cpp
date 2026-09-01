#include "net/wifi_service.h"

#include <Arduino.h>
#include <WiFi.h>

#if __has_include("../../secrets.h")
#include "../../secrets.h"
#else
#include "../../secrets.example.h"
#endif

void WifiService::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  _lastAttemptMs = 0;
}

void WifiService::tick() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!_connected) {
      Serial.printf("WiFi OK: %s\n", WiFi.localIP().toString().c_str());
    }
    _connected = true;
    return;
  }

  _connected = false;
  unsigned long now = millis();
  if (_lastAttemptMs != 0 && now - _lastAttemptMs < 10000) {
    return;
  }
  _lastAttemptMs = now;
  Serial.printf("WiFi connecting to %s...\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}
