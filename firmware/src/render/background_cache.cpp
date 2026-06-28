#include "background_cache.h"

#include <Arduino.h>
#include <cstring>

BackgroundCache::~BackgroundCache() {
  if (_pixels) {
    free(_pixels);
    _pixels = nullptr;
  }
}

void BackgroundCache::reset() {
  _valid = false;
  _w = 0;
  _h = 0;
}

bool BackgroundCache::ensureCapacity(size_t pixelCount) {
  if (pixelCount == 0) {
    return false;
  }
  if (_capacity >= pixelCount && _pixels) {
    return true;
  }
  uint16_t *next = static_cast<uint16_t *>(realloc(_pixels, pixelCount * sizeof(uint16_t)));
  if (!next) {
    return false;
  }
  _pixels = next;
  _capacity = pixelCount;
  return true;
}

bool BackgroundCache::captureFromBackground(const uint16_t *bgPixels, int bgW, int bgH,
                                            int patchX, int patchY, int patchW, int patchH) {
  if (!bgPixels || bgW <= 0 || bgH <= 0 || patchW <= 0 || patchH <= 0) {
    _valid = false;
    return false;
  }

  int srcX = patchX;
  int srcY = patchY;
  if (srcX < 0) {
    patchW += srcX;
    srcX = 0;
  }
  if (srcY < 0) {
    patchH += srcY;
    srcY = 0;
  }
  if (srcX + patchW > bgW) {
    patchW = bgW - srcX;
  }
  if (srcY + patchH > bgH) {
    patchH = bgH - srcY;
  }
  if (patchW <= 0 || patchH <= 0) {
    _valid = false;
    return false;
  }

  if (!ensureCapacity(static_cast<size_t>(patchW) * static_cast<size_t>(patchH))) {
    _valid = false;
    return false;
  }

  _x = srcX;
  _y = srcY;
  _w = patchW;
  _h = patchH;

  for (int row = 0; row < patchH; ++row) {
    const uint16_t *srcRow = bgPixels + (srcY + row) * bgW + srcX;
    uint16_t *dstRow = _pixels + row * patchW;
    memcpy(dstRow, srcRow, static_cast<size_t>(patchW) * sizeof(uint16_t));
  }

  _valid = true;
  return true;
}

void BackgroundCache::restore(IRenderer &renderer) const {
  if (!_valid || !_pixels || _w <= 0 || _h <= 0) {
    return;
  }
  renderer.blitRGB565(_pixels, _x, _y, _w, _h);
}
