#include "engine.h"

void AnimationEngine::begin() {
  _animation = "idle";
  _lastTick = millis();
}

void AnimationEngine::tick(unsigned long nowMs) {
  if (nowMs - _lastTick >= 500) {
    _blink = !_blink;
    _lastTick = nowMs;
  }
}

void AnimationEngine::setAnimation(const std::string &name) {
  if (!name.empty()) {
    _animation = name;
  }
}
