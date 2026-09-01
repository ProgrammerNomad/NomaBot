#include "net/clock_service.h"

#include <Arduino.h>
#include <time.h>

#include "character/character_runtime.h"
#include "net/wifi_service.h"

#if __has_include("../../secrets.h")
#include "../../secrets.h"
#else
#include "../../secrets.example.h"
#endif

extern WifiService gWifiService;

void ClockService::begin(CharacterRuntime *runtime) {
  _runtime = runtime;
  _ntpSynced = false;
  _lastSyncMs = 0;
  _lastUpdateMs = 0;
}

void ClockService::syncNtp() {
  if (!gWifiService.connected()) {
    return;
  }
  unsigned long now = millis();
  if (_ntpSynced && now - _lastSyncMs < 3600000UL) {
    return;
  }
  configTime(TIMEZONE_OFFSET_SEC, 0, "pool.ntp.org", "time.nist.gov");
  _lastSyncMs = now;
  _ntpSynced = true;
  Serial.println("NTP configured");
}

void ClockService::updateClock() {
  if (!_runtime) {
    return;
  }
  time_t nowSec = time(nullptr);
  if (nowSec < 100000) {
    _runtime->setClock("--:--", nullptr);
    return;
  }
  struct tm timeInfo;
  localtime_r(&nowSec, &timeInfo);
  char timeBuf[8];
  strftime(timeBuf, sizeof(timeBuf), "%H:%M", &timeInfo);
  char dateBuf[16];
  strftime(dateBuf, sizeof(dateBuf), "%a %d %b", &timeInfo);
  _timeText = timeBuf;
  _dateText = dateBuf;
  _runtime->setClock(_timeText.c_str(), _dateText.c_str());
}

void ClockService::tick() {
  syncNtp();
  unsigned long now = millis();
  if (now - _lastUpdateMs < 1000) {
    return;
  }
  _lastUpdateMs = now;
  updateClock();
}
