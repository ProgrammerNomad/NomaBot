#include "net/calendar_service.h"

#include <Arduino.h>
#include <string.h>

void CalendarService::begin() { _text[0] = '\0'; }

void CalendarService::tick() {
  unsigned long now = millis();
  if (_lastFetchMs != 0 && now - _lastFetchMs < 300000UL) {
    return;
  }
  _lastFetchMs = now;
  strncpy(_text, "Calendar: configure in portal", sizeof(_text) - 1);
  _text[sizeof(_text) - 1] = '\0';
}

const char *CalendarService::overlayText() const { return _text; }
