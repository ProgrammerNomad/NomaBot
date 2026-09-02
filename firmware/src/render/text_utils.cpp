#include "render/text_utils.h"

#include "renderer/renderer.hpp"

namespace {
constexpr uint16_t kClearColor = 0x0000;
constexpr uint16_t kTextColor = 0xFFFF;
constexpr uint16_t kAccentColor = 0x07FF;
}  // namespace

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

void drawScaledCenteredLine(IRenderer &renderer, const char *text, int y, uint16_t color,
                            uint8_t scale) {
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
  renderer.fillScreen(kClearColor);
  int w = renderer.width();
  int h = renderer.height();
  const bool threeLines = line3 && line3[0];
  if (line1 && line1[0]) {
    uint8_t scale1 = fitTextScale(line1, w, threeLines ? 7 : 8);
    int lineH1 = 8 * scale1;
    int y1 = threeLines ? h / 6 : static_cast<int>(h * 0.28f);
    drawScaledCenteredLine(renderer, line1, y1, kAccentColor, scale1);
    if (line2 && line2[0]) {
      uint8_t scale2 = fitTextScale(line2, w, 3);
      int y2 = threeLines ? y1 + lineH1 + 8 : static_cast<int>(h * 0.62f);
      drawScaledCenteredLine(renderer, line2, y2, kTextColor, scale2);
    }
    if (line3 && line3[0]) {
      uint8_t scale3 = fitTextScale(line3, w, 2);
      int y3 = static_cast<int>(h * 0.72f);
      drawScaledCenteredLine(renderer, line3, y3, kTextColor, scale3);
    }
    return;
  }
  if (line2 && line2[0]) {
    uint8_t scale2 = fitTextScale(line2, w, 3);
    drawScaledCenteredLine(renderer, line2, h / 2 - 12, kTextColor, scale2);
  }
}
