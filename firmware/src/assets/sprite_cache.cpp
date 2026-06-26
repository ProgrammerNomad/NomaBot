#include "sprite_cache.h"

#include <Arduino.h>
#include <cstring>
#include <esp_heap_caps.h>

SpriteCache::SpriteCache() = default;

SpriteCache::~SpriteCache() { clear(); }

void SpriteCache::clear() {
  for (auto &e : _entries) {
    _free(e);
  }
  _next = 0;
}

void SpriteCache::_free(CachedSprite &slot) {
  if (slot.pixels) {
    heap_caps_free(slot.pixels);
    slot.pixels = nullptr;
  }
  slot.valid = false;
  slot.id[0] = '\0';
}

CachedSprite *SpriteCache::_find(const char *spriteId) {
  for (auto &e : _entries) {
    if (e.valid && strncmp(e.id, spriteId, sizeof(e.id)) == 0) {
      return &e;
    }
  }
  return nullptr;
}

bool SpriteCache::_load(CachedSprite &slot, const char *spriteId, PackLoader &loader,
                        const SpriteMeta *meta) {
  _free(slot);
  if (!meta) {
    return false;
  }

  size_t count = static_cast<size_t>(meta->width) * static_cast<size_t>(meta->height);
  size_t bytes = count * sizeof(uint16_t);
  slot.pixels = static_cast<uint16_t *>(heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (!slot.pixels) {
    slot.pixels = static_cast<uint16_t *>(malloc(bytes));
  }
  if (!slot.pixels) {
    return false;
  }

  if (!loader.readSpritePixels(meta, slot.pixels, bytes)) {
    _free(slot);
    return false;
  }

  strncpy(slot.id, spriteId, sizeof(slot.id) - 1);
  slot.id[sizeof(slot.id) - 1] = '\0';
  slot.width = meta->width;
  slot.height = meta->height;
  slot.valid = true;
  return true;
}

const uint16_t *SpriteCache::get(const char *spriteId, PackLoader &loader, const SpriteMeta *meta) {
  if (!spriteId || !meta) {
    return nullptr;
  }

  CachedSprite *hit = _find(spriteId);
  if (hit) {
    return hit->pixels;
  }

  CachedSprite &slot = _entries[_next];
  _next = (_next + 1) % kMaxEntries;
  if (!_load(slot, spriteId, loader, meta)) {
    return nullptr;
  }
  return slot.pixels;
}
