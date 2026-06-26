#include "compositor.h"

#include "assets/asset_registry.h"

void Compositor::blitSprite(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                            const char *spriteId, int x, int y) {
  const SpriteMeta *meta = loader.findSprite(spriteId);
  if (!meta) {
    return;
  }
  const uint16_t *pixels = cache.get(spriteId, loader, meta);
  if (!pixels) {
    return;
  }
  int drawX = x - meta->width / 2;
  int drawY = y;
  renderer.blitRGB565(pixels, drawX, drawY, meta->width, meta->height);
}

void Compositor::render(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                        AssetRegistry &assets, const char *backgroundSpriteId,
                        const char *bodySpriteId, int anchorX, int anchorY, const char *messageText,
                        const char *animationLabel) {
  (void)assets;

  if (backgroundSpriteId && backgroundSpriteId[0]) {
    const SpriteMeta *bgMeta = loader.findSprite(backgroundSpriteId);
    if (bgMeta) {
      const uint16_t *bgPixels = cache.get(backgroundSpriteId, loader, bgMeta);
      if (bgPixels) {
        renderer.blitRGB565(bgPixels, 0, 0, bgMeta->width, bgMeta->height);
      }
    }
  } else {
    renderer.fillScreen(0x0000);
  }

  if (bodySpriteId && bodySpriteId[0]) {
    blitSprite(renderer, loader, cache, bodySpriteId, anchorX, anchorY);
  }

  if (animationLabel && animationLabel[0]) {
    renderer.drawText(4, 8, animationLabel, 0xFFFF);
  }

  if (messageText && messageText[0]) {
    renderer.drawText(4, renderer.height() - 20, messageText, 0xFFFF);
  }
}
