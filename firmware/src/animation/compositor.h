#pragma once

#include "assets/pack_loader.h"
#include "renderer/renderer.hpp"
#include "assets/sprite_cache.h"

class AssetRegistry;

class Compositor {
public:
  void render(IRenderer &renderer, PackLoader &loader, SpriteCache &cache, AssetRegistry &assets,
              const char *backgroundSpriteId, const char *bodySpriteId, int anchorX, int anchorY,
              const char *messageText, const char *animationLabel);

  void blitSprite(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                  const char *spriteId, int x, int y, bool colorKey = false);
};
