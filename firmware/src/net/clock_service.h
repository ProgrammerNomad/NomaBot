#pragma once

#include <string>

class CharacterRuntime;

class ClockService {
public:
  void begin(CharacterRuntime *runtime);
  void tick();
  bool clockValid() const { return _clockValid; }
  int currentHour() const { return _currentHour; }

private:
  void syncNtp();
  void updateClock();

  CharacterRuntime *_runtime = nullptr;
  bool _ntpSynced = false;
  bool _clockValid = false;
  int _currentHour = 0;
  unsigned long _lastSyncMs = 0;
  unsigned long _lastUpdateMs = 0;
  std::string _timeText;
  std::string _dateText;
};
