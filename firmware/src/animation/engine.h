#pragma once

#include <Arduino.h>
#include <string>

class AnimationEngine {
public:
  void begin();
  void tick(unsigned long nowMs);
  void setAnimation(const std::string &name);
  const std::string &currentAnimation() const { return _animation; }
  bool blinkPhase() const { return _blink; }
  void setMessage(const std::string &msg) { _message = msg; }
  const std::string &message() const { return _message; }

private:
  std::string _animation = "idle";
  std::string _message;
  unsigned long _lastTick = 0;
  bool _blink = false;
};
