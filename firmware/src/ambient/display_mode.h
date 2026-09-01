#pragma once

#include <cstdint>

enum class AmbientDisplayMode : uint8_t {
  EyesAnim = 0,
  ClockScreen = 1,
  WeatherScreen = 2,
};

class DisplayModeController {
public:
  void begin();
  void tick(unsigned long nowMs);
  AmbientDisplayMode current() const { return _mode; }
  bool isEyesMode() const { return _mode == AmbientDisplayMode::EyesAnim; }
  bool consumeChanged();

private:
  void advanceMode();
  unsigned long durationMs(AmbientDisplayMode mode) const;

  AmbientDisplayMode _mode = AmbientDisplayMode::EyesAnim;
  unsigned long _modeStartMs = 0;
  unsigned long _lastBtnMs = 0;
  bool _btnDown = false;
  bool _changed = false;
};
