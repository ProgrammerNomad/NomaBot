#include "render/screen_renderers.h"

#include <stdio.h>

#include "render/text_utils.h"

namespace {
constexpr uint16_t kHudClearColor = 0x0000;
}  // namespace


void drawServiceStatusDots(IRenderer &renderer, const ServiceStatus &status) {
  int x = renderer.width() - 14;
  int y = 4;
  uint16_t wifiColor = status.wifiConnected ? 0x07E0 : (status.wifiReconnecting ? 0xFD20 : 0xF800);
  uint16_t clockColor = status.clockValid ? 0x07E0 : 0xF800;
  uint16_t weatherColor =
      status.weatherOk && !status.weatherStale ? 0x07E0 : (status.weatherStale ? 0xFD20 : 0xF800);
  renderer.fillRect(x, y, 4, 4, wifiColor);
  renderer.fillRect(x - 6, y, 4, 4, clockColor);
  renderer.fillRect(x - 12, y, 4, 4, weatherColor);
}

void drawModeTransitionFade(IRenderer &renderer, uint8_t alpha) {
  if (alpha == 0) {
    return;
  }
  uint16_t shade = alpha >> 3;
  renderer.fillRect(0, 0, renderer.width(), renderer.height(), shade);
}

void drawClockScreen(IRenderer &renderer, const RenderState &state, const ServiceStatus &status) {
  const char *line1 = state.clockText;
  const char *line2 = state.clockDateText;
  if (status.wifiReconnecting) {
    line1 = "WiFi...";
    line2 = nullptr;
  }
  drawLargeCenteredText(renderer, line1, line2, nullptr);
  drawServiceStatusDots(renderer, status);
}

void drawWeatherScreen(IRenderer &renderer, const RenderState &state, const ServiceStatus &status) {
  drawLargeCenteredText(renderer, state.weatherText, state.weatherConditionText,
                        state.weatherCityText);
  drawServiceStatusDots(renderer, status);
}

void drawPomodoroScreen(IRenderer &renderer, const RenderState &state) {
  renderer.fillScreen(kHudClearColor);
  char line1[16];
  unsigned long mins = state.pomodoroRemainingSec / 60;
  unsigned long secs = state.pomodoroRemainingSec % 60;
  snprintf(line1, sizeof(line1), "%02lu:%02lu", mins, secs);
  drawLargeCenteredText(renderer, line1, "Pomodoro", nullptr);
  int cx = renderer.width() / 2;
  int cy = renderer.height() / 2 + 20;
  int radius = 28;
  float progress = state.pomodoroTotalSec > 0
                       ? 1.0f - static_cast<float>(state.pomodoroRemainingSec) /
                                     static_cast<float>(state.pomodoroTotalSec)
                       : 0.0f;
  int arcLen = static_cast<int>(progress * radius * 2);
  renderer.fillRect(cx - radius, cy - 2, arcLen, 4, 0x07FF);
}

void drawStatsScreen(IRenderer &renderer, const RenderState &state) {
  renderer.fillScreen(kHudClearColor);
  char line1[32];
  char line2[32];
  char line3[32];
  snprintf(line1, sizeof(line1), "Up %lus", state.uptimeSec);
  snprintf(line2, sizeof(line2), "Heap %lu RSSI %d", state.heapFree, state.wifiRssi);
  snprintf(line3, sizeof(line3), "FW %s", state.firmwareVersion ? state.firmwareVersion : "?");
  drawLargeCenteredText(renderer, line1, line2, line3);
}
