#include "emotion_state.h"

#include <Arduino.h>
#include <string.h>

void EmotionState::setEmotion(const char *emotion) {
  if (emotion && emotion[0]) {
    _emotion = emotion;
    _setAtMs = millis();
  }
}

void EmotionState::tick(unsigned long nowMs) {
  if (_emotion == "neutral") {
    return;
  }
  if (_setAtMs == 0) {
    _setAtMs = nowMs;
    return;
  }
  if (nowMs - _setAtMs >= kDecayMs) {
    _emotion = "neutral";
    _setAtMs = 0;
  }
}
