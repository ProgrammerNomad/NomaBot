#include "net/wifi_service.h"

#include <Arduino.h>
#include <WiFi.h>

#include "net/device_config.h"

void WifiService::begin() {
  const DeviceConfig &cfg = deviceConfig();
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  _connected = false;
  _reconnecting = false;
  _lastAttemptMs = 0;
  _backoffMs = 10000;
  _attemptCount = 0;
  if (cfg.wifiSsid[0] && strcmp(cfg.wifiSsid, "your_wifi_ssid") != 0) {
    Serial.printf("WiFi connecting to %s...\n", cfg.wifiSsid);
    WiFi.begin(cfg.wifiSsid, cfg.wifiPass);
    _lastAttemptMs = millis();
  }
}

void WifiService::tick() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!_connected) {
      Serial.printf("WiFi OK: %s RSSI=%d\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
    }
    _connected = true;
    _reconnecting = false;
    _backoffMs = 10000;
    _attemptCount = 0;
    return;
  }

  _connected = false;
  _reconnecting = _attemptCount > 0 || _lastAttemptMs != 0;
  unsigned long now = millis();
  if (_lastAttemptMs != 0 && now - _lastAttemptMs < _backoffMs) {
    return;
  }
  const DeviceConfig &cfg = deviceConfig();
  if (!cfg.wifiSsid[0] || strcmp(cfg.wifiSsid, "your_wifi_ssid") == 0) {
    return;
  }
  _lastAttemptMs = now;
  _attemptCount++;
  Serial.printf("WiFi reconnect attempt %u (backoff %lums)\n", _attemptCount, _backoffMs);
  WiFi.disconnect();
  WiFi.begin(cfg.wifiSsid, cfg.wifiPass);
  if (_backoffMs < 300000UL) {
    _backoffMs *= 2;
    if (_backoffMs > 300000UL) {
      _backoffMs = 300000UL;
    }
  }
}

int WifiService::rssi() const {
  if (!_connected) {
    return 0;
  }
  return WiFi.RSSI();
}
