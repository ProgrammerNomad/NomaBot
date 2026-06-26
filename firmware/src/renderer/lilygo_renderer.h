#pragma once

#include <cstdint>

#include "renderer.hpp"

class LilygoRenderer : public IRenderer {
public:
  bool begin() override;
  void fillScreen(uint16_t color) override;
  void blitRGB565(const uint16_t *pixels, int x, int y, int w, int h) override;
  void drawText(int x, int y, const char *text, uint16_t color) override;
  int width() const override;
  int height() const override;

  void fillRect(int x, int y, int w, int h, uint16_t color);
};
