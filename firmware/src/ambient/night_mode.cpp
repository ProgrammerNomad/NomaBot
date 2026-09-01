#include "ambient/night_mode.h"

#include "net/device_config.h"
#include "renderer/lilygo_renderer.h"

void NightModeController::begin(LilygoRenderer *renderer) { _renderer = renderer; }

void NightModeController::tick(int currentHour) {
  if (!_renderer) {
    return;
  }
  const DeviceConfig &cfg = deviceConfig();
  bool night = false;
  if (cfg.nightStartHour < cfg.nightEndHour) {
    night = currentHour >= cfg.nightStartHour && currentHour < cfg.nightEndHour;
  } else {
    night = currentHour >= cfg.nightStartHour || currentHour < cfg.nightEndHour;
  }
  if (night == _nightActive) {
    return;
  }
  _nightActive = night;
  _renderer->setBrightness(night ? static_cast<uint8_t>(cfg.nightBrightness)
                                 : static_cast<uint8_t>(cfg.dayBrightness));
}
