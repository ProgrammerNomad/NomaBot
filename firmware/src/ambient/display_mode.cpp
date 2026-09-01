#include "ambient/display_mode.h"

#include <Arduino.h>

static constexpr int kPinUserBtn = 14;
static constexpr unsigned long kDebounceMs = 200;
static constexpr unsigned long kDurEyesMs = 45000;
static constexpr unsigned long kDurClockMs = 8000;
static constexpr unsigned long kDurWeatherMs = 8000;

void DisplayModeController::begin() {
  pinMode(kPinUserBtn, INPUT_PULLUP);
  _mode = AmbientDisplayMode::EyesAnim;
  _modeStartMs = millis();
  _lastBtnMs = 0;
  _btnDown = false;
  _changed = true;
}

unsigned long DisplayModeController::durationMs(AmbientDisplayMode mode) const {
  switch (mode) {
  case AmbientDisplayMode::EyesAnim:
    return kDurEyesMs;
  case AmbientDisplayMode::ClockScreen:
    return kDurClockMs;
  case AmbientDisplayMode::WeatherScreen:
    return kDurWeatherMs;
  }
  return kDurEyesMs;
}

void DisplayModeController::advanceMode() {
  switch (_mode) {
  case AmbientDisplayMode::EyesAnim:
    _mode = AmbientDisplayMode::ClockScreen;
    break;
  case AmbientDisplayMode::ClockScreen:
    _mode = AmbientDisplayMode::WeatherScreen;
    break;
  case AmbientDisplayMode::WeatherScreen:
    _mode = AmbientDisplayMode::EyesAnim;
    break;
  }
  _changed = true;
}

void DisplayModeController::tick(unsigned long nowMs) {
  bool pressed = digitalRead(kPinUserBtn) == LOW;
  if (pressed && !_btnDown && nowMs - _lastBtnMs >= kDebounceMs) {
    _btnDown = true;
    _lastBtnMs = nowMs;
    advanceMode();
    _modeStartMs = nowMs;
    return;
  }
  if (!pressed) {
    _btnDown = false;
  }

  if (nowMs - _modeStartMs >= durationMs(_mode)) {
    advanceMode();
    _modeStartMs = nowMs;
  }
}

bool DisplayModeController::consumeChanged() {
  if (!_changed) {
    return false;
  }
  _changed = false;
  return true;
}
