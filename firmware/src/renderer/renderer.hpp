#pragma once

#include <cstdint>

// RGB565 magenta — not used in NomaBot palette; marks transparent pixels in compiled sprites.
constexpr uint16_t kSpriteColorKey = 0xF81F;

class IRenderer {
public:
  virtual ~IRenderer() = default;
  virtual bool begin() = 0;
  virtual void fillScreen(uint16_t color) = 0;
  virtual void fillRect(int x, int y, int w, int h, uint16_t color) = 0;
  virtual void blitRGB565(const uint16_t *pixels, int x, int y, int w, int h) = 0;
  virtual void blitRGB565ColorKey(const uint16_t *pixels, int x, int y, int w, int h,
                                  uint16_t colorKey) = 0;
  virtual void drawText(int x, int y, const char *text, uint16_t color) = 0;
  virtual int width() const = 0;
  virtual int height() const = 0;
};
