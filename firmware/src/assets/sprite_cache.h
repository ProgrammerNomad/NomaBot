#pragma once

#include <cstdint>

#include "assets/pack_loader.h"

struct CachedSprite {
  char id[32];
  uint16_t *pixels = nullptr;
  int width = 0;
  int height = 0;
  bool valid = false;
};

class SpriteCache {
public:
  static constexpr int kMaxEntries = 4;

  SpriteCache();
  ~SpriteCache();

  void clear();
  const uint16_t *get(const char *spriteId, PackLoader &loader, const SpriteMeta *meta);

private:
  CachedSprite _entries[kMaxEntries];
  int _next = 0;

  CachedSprite *_find(const char *spriteId);
  bool _load(CachedSprite &slot, const char *spriteId, PackLoader &loader, const SpriteMeta *meta);
  void _free(CachedSprite &slot);
};
