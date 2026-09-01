#include "ambient/display_mode.h"

#include <Arduino.h>

#include "net/device_config.h"

static constexpr int kPinUserBtn = 14;
static constexpr unsigned long kDebounceMs = 200;
static constexpr unsigned long kLongPressMs = 800;

void DisplayModeController::begin() {
  pinMode(kPinUserBtn, INPUT_PULLUP);
  _mode = AmbientDisplayMode::EyesAnim;
  _modeStartMs = millis();
  _btnDownMs = 0;
  _btnDown = false;
  _changed = true;
}

unsigned long DisplayModeController::durationMs(AmbientDisplayMode mode) const {
  const DeviceConfig &cfg = deviceConfig();
  switch (mode) {
  case AmbientDisplayMode::EyesAnim:
    return cfg.eyesDurationMs;
  case AmbientDisplayMode::ClockScreen:
    return cfg.clockDurationMs;
  case AmbientDisplayMode::WeatherScreen:
    return cfg.weatherDurationMs;
  case AmbientDisplayMode::PomodoroScreen:
  case AmbientDisplayMode::StatsScreen:
    return 0;
  }
  return cfg.eyesDurationMs;
}

void DisplayModeController::setMode(AmbientDisplayMode mode, unsigned long nowMs) {
  if (_mode == mode) {
    return;
  }
  _mode = mode;
  _modeStartMs = nowMs;
  _changed = true;
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
  case AmbientDisplayMode::PomodoroScreen:
  case AmbientDisplayMode::StatsScreen:
    _mode = AmbientDisplayMode::EyesAnim;
    break;
  }
  _changed = true;
}

DisplayModeInput DisplayModeController::tick(unsigned long nowMs) {
  DisplayModeInput input;
  bool pressed = digitalRead(kPinUserBtn) == LOW;
  if (pressed && !_btnDown) {
    _btnDown = true;
    _btnDownMs = nowMs;
  }
  if (!pressed && _btnDown) {
    _btnDown = false;
    unsigned long held = nowMs - _btnDownMs;
    if (held >= kLongPressMs) {
      input.longPress = true;
      if (_mode == AmbientDisplayMode::EyesAnim) {
        setMode(AmbientDisplayMode::PomodoroScreen, nowMs);
      } else if (_mode != AmbientDisplayMode::PomodoroScreen) {
        setMode(AmbientDisplayMode::StatsScreen, nowMs);
      }
    } else if (held >= kDebounceMs) {
      input.shortPress = true;
      if (_mode == AmbientDisplayMode::PomodoroScreen) {
        // Pomodoro handled by caller
      } else if (_mode == AmbientDisplayMode::StatsScreen) {
        setMode(AmbientDisplayMode::EyesAnim, nowMs);
      } else {
        advanceMode();
        _modeStartMs = nowMs;
      }
    }
  }

  if (_mode == AmbientDisplayMode::PomodoroScreen || _mode == AmbientDisplayMode::StatsScreen) {
    return input;
  }
  unsigned long dur = durationMs(_mode);
  if (dur > 0 && nowMs - _modeStartMs >= dur) {
    advanceMode();
    _modeStartMs = nowMs;
  }
  return input;
}

bool DisplayModeController::consumeChanged() {
  if (!_changed) {
    return false;
  }
  _changed = false;
  return true;
}
