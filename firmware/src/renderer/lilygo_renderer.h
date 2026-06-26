#pragma once

#include <cstdint>

class LilygoRenderer {
public:
  bool begin();
  void beginFrame();
  void fillScreen(uint16_t color);
  void fillRect(int x, int y, int w, int h, uint16_t color);
  void drawText(int x, int y, const char *text, uint16_t color);
  void endFrame();
  int width() const;
  int height() const;
};
