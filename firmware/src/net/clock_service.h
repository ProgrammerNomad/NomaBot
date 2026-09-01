#pragma once

#include <string>

class CharacterRuntime;

class ClockService {
public:
  void begin(CharacterRuntime *runtime);
  void tick();

private:
  void syncNtp();
  void updateClock();

  CharacterRuntime *_runtime = nullptr;
  bool _ntpSynced = false;
  unsigned long _lastSyncMs = 0;
  unsigned long _lastUpdateMs = 0;
  std::string _timeText;
  std::string _dateText;
};
