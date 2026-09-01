#include "render/screen_renderers.h"

#include <stdio.h>

namespace {

constexpr uint16_t kHudClearColor = 0x0000;
constexpr uint16_t kTextColor = 0xFFFF;

int textWidthEstimate(const char *text) {
  if (!text) {
    return 0;
  }
  int len = 0;
  while (text[len]) {
    len++;
  }
  return len * 6;
}

uint8_t fitTextScale(const char *text, int maxWidth, uint8_t preferredScale) {
  if (!text || !text[0] || maxWidth <= 16) {
    return 1;
  }
  int len = 0;
  while (text[len]) {
    len++;
  }
  int maxScale = (maxWidth - 16) / (len * 6);
  if (maxScale < 1) {
    return 1;
  }
  if (maxScale > 255) {
    maxScale = 255;
  }
  if (preferredScale > static_cast<uint8_t>(maxScale)) {
    return static_cast<uint8_t>(maxScale);
  }
  return preferredScale > 0 ? preferredScale : 1;
}

void drawScaledLine(IRenderer &renderer, const char *text, int y, uint16_t color, uint8_t scale) {
  if (!text || !text[0]) {
    return;
  }
  int tw = textWidthEstimate(text) * scale;
  int x = (renderer.width() - tw) / 2;
  if (x < 4) {
    x = 4;
  }
  renderer.drawTextScale(x, y, text, color, scale);
}

void drawLargeCenteredText(IRenderer &renderer, const char *line1, const char *line2,
                           const char *line3) {
  renderer.fillScreen(kHudClearColor);
  int w = renderer.width();
  int h = renderer.height();
  const bool threeLines = line3 && line3[0];
  if (line1 && line1[0]) {
    uint8_t scale1 = fitTextScale(line1, w, threeLines ? 7 : 8);
    int lineH1 = 8 * scale1;
    int y1 = threeLines ? h / 6 : static_cast<int>(h * 0.28f);
    drawScaledLine(renderer, line1, y1, 0x07FF, scale1);
    if (line2 && line2[0]) {
      uint8_t scale2 = fitTextScale(line2, w, threeLines ? 3 : 3);
      int y2 = threeLines ? y1 + lineH1 + 8 : static_cast<int>(h * 0.62f);
      drawScaledLine(renderer, line2, y2, kTextColor, scale2);
    }
    if (line3 && line3[0]) {
      uint8_t scale3 = fitTextScale(line3, w, 2);
      int y3 = static_cast<int>(h * 0.72f);
      drawScaledLine(renderer, line3, y3, kTextColor, scale3);
    }
    return;
  }
  if (line2 && line2[0]) {
    uint8_t scale2 = fitTextScale(line2, w, 3);
    drawScaledLine(renderer, line2, h / 2 - 12, kTextColor, scale2);
  }
}

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
