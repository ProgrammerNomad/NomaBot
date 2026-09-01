#pragma once

class CalendarService {
public:
  void begin();
  void tick();
  const char *overlayText() const;

private:
  char _text[64];
  unsigned long _lastFetchMs = 0;
};
