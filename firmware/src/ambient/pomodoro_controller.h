#pragma once

#include <cstdint>

enum class PomodoroState : uint8_t {
  Idle = 0,
  Running,
  Paused,
  Complete,
};

class PomodoroController {
public:
  void begin();
  void tick(unsigned long nowMs);
  void toggleStartPause(unsigned long nowMs);
  void reset();
  PomodoroState state() const { return _state; }
  unsigned long remainingSec() const { return _remainingSec; }
  unsigned long totalSec() const { return _totalSec; }
  bool consumeCompleted();

private:
  PomodoroState _state = PomodoroState::Idle;
  unsigned long _totalSec = 25 * 60;
  unsigned long _remainingSec = 25 * 60;
  unsigned long _lastTickMs = 0;
  bool _completedFlag = false;
};
