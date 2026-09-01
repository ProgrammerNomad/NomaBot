#include "net/clock_service.h"

#include <Arduino.h>
#include <time.h>

#include "character/character_runtime.h"
#include "net/device_config.h"
#include "net/wifi_service.h"

extern WifiService gWifiService;

void ClockService::begin(CharacterRuntime *runtime) {
  _runtime = runtime;
  _ntpSynced = false;
  _clockValid = false;
  _currentHour = 0;
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
  const DeviceConfig &cfg = deviceConfig();
  setenv("TZ", cfg.timezone, 1);
  tzset();
#if defined(TIMEZONE_OFFSET_SEC)
  configTime(TIMEZONE_OFFSET_SEC, 0, "pool.ntp.org", "time.nist.gov");
#else
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
#endif
  _lastSyncMs = now;
  _ntpSynced = true;
  Serial.printf("NTP configured TZ=%s\n", cfg.timezone);
}

void ClockService::updateClock() {
  if (!_runtime) {
    return;
  }
  time_t nowSec = time(nullptr);
  if (nowSec < 100000) {
    _clockValid = false;
    _runtime->setClock("--:--", nullptr);
    return;
  }
  struct tm timeInfo;
  localtime_r(&nowSec, &timeInfo);
  _currentHour = timeInfo.tm_hour;
  _clockValid = true;
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
