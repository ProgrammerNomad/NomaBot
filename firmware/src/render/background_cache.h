#pragma once

#include <cstddef>
#include <cstdint>

#include "renderer/renderer.hpp"

class BackgroundCache {
public:
  ~BackgroundCache();

  void reset();
  bool valid() const { return _valid; }
  int x() const { return _x; }
  int y() const { return _y; }
  int width() const { return _w; }
  int height() const { return _h; }

  bool captureFromBackground(const uint16_t *bgPixels, int bgW, int bgH, int patchX, int patchY,
                             int patchW, int patchH);
  void restore(IRenderer &renderer) const;

private:
  uint16_t *_pixels = nullptr;
  size_t _capacity = 0;
  int _x = 0;
  int _y = 0;
  int _w = 0;
  int _h = 0;
  bool _valid = false;

  bool ensureCapacity(size_t pixelCount);
};
