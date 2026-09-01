#pragma once

#include <cstdint>

class MinigameController {
public:
  void begin();
  void onButtonPress(unsigned long nowMs);
  void tick(unsigned long nowMs);
  const char *statusText() const;

private:
  enum class Phase : uint8_t { Idle, Waiting, Go, Result };
  Phase _phase = Phase::Idle;
  unsigned long _phaseStartMs = 0;
  unsigned long _reactionMs = 0;
  char _text[32];
};
