#pragma once

#include <cstdint>

enum class AmbientDisplayMode : uint8_t {
  EyesAnim = 0,
  ClockScreen = 1,
  WeatherScreen = 2,
  PomodoroScreen = 3,
  StatsScreen = 4,
};

struct DisplayModeInput {
  bool shortPress = false;
  bool longPress = false;
};

class DisplayModeController {
public:
  void begin();
  DisplayModeInput tick(unsigned long nowMs);
  AmbientDisplayMode current() const { return _mode; }
  bool isEyesMode() const { return _mode == AmbientDisplayMode::EyesAnim; }
  bool consumeChanged();
  void setMode(AmbientDisplayMode mode, unsigned long nowMs);

private:
  void advanceMode();
  unsigned long durationMs(AmbientDisplayMode mode) const;

  AmbientDisplayMode _mode = AmbientDisplayMode::EyesAnim;
  unsigned long _modeStartMs = 0;
  unsigned long _btnDownMs = 0;
  bool _btnDown = false;
  bool _changed = false;
};
