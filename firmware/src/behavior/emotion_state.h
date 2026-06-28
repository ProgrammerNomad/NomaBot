#pragma once

#include <string>

class EmotionState {
public:
  void setEmotion(const char *emotion);
  void tick(unsigned long nowMs);
  const char *current() const { return _emotion.c_str(); }

private:
  std::string _emotion = "neutral";
  unsigned long _setAtMs = 0;
  static constexpr unsigned long kDecayMs = 300000;
};
