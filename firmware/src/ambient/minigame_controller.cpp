#include "ambient/minigame_controller.h"

#include <Arduino.h>
#include <stdio.h>

void MinigameController::begin() {
  _phase = Phase::Idle;
  _text[0] = '\0';
}

void MinigameController::onButtonPress(unsigned long nowMs) {
  if (_phase == Phase::Idle) {
    _phase = Phase::Waiting;
    _phaseStartMs = nowMs;
    snprintf(_text, sizeof(_text), "Wait for green...");
    return;
  }
  if (_phase == Phase::Go) {
    _reactionMs = nowMs - _phaseStartMs;
    _phase = Phase::Result;
    snprintf(_text, sizeof(_text), "React: %lums", _reactionMs);
    return;
  }
  if (_phase == Phase::Result) {
    _phase = Phase::Idle;
    _text[0] = '\0';
  }
}

void MinigameController::tick(unsigned long nowMs) {
  if (_phase == Phase::Waiting) {
    if (nowMs - _phaseStartMs > 2000UL) {
      _phase = Phase::Go;
      _phaseStartMs = nowMs;
      snprintf(_text, sizeof(_text), "PRESS!");
    }
  }
}

const char *MinigameController::statusText() const {
  return _text[0] ? _text : nullptr;
}
