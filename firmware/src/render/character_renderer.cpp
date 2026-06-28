#include "character_renderer.h"

namespace {

constexpr int kHudBandHeight = 18;
constexpr int kBubbleBandHeight = 24;
constexpr uint16_t kHudClearColor = 0x0000;
constexpr uint16_t kTextColor = 0xFFFF;

bool captureCharacterFootprint(PackLoader &loader, SpriteCache &cache, BackgroundCache &bgCache,
                               const SceneNode &character, const char *bgSpriteId) {
  if (!character.visible || !character.spriteId || !bgSpriteId || !bgSpriteId[0]) {
    bgCache.reset();
    return false;
  }

  const SpriteMeta *bgMeta = loader.findSprite(bgSpriteId);
  const SpriteMeta *bodyMeta = loader.findSprite(character.spriteId);
  if (!bgMeta || !bodyMeta) {
    bgCache.reset();
    return false;
  }

  const uint16_t *bgPixels = cache.get(bgSpriteId, loader, bgMeta);
  if (!bgPixels) {
    bgCache.reset();
    return false;
  }

  int drawX = character.x - bodyMeta->width / 2;
  int drawY = character.y;
  return bgCache.captureFromBackground(bgPixels, bgMeta->width, bgMeta->height, drawX, drawY,
                                       bodyMeta->width, bodyMeta->height);
}

void drawBackgroundNode(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                        Compositor &compositor, BackgroundCache &bgCache,
                        const SceneNode &node, const SceneNode &character) {
  if (!node.visible || !node.spriteId || !node.spriteId[0]) {
    renderer.fillScreen(kHudClearColor);
    bgCache.reset();
    return;
  }

  const SpriteMeta *bgMeta = loader.findSprite(node.spriteId);
  if (!bgMeta) {
    renderer.fillScreen(kHudClearColor);
    bgCache.reset();
    return;
  }

  const uint16_t *bgPixels = cache.get(node.spriteId, loader, bgMeta);
  if (!bgPixels) {
    renderer.fillScreen(kHudClearColor);
    bgCache.reset();
    return;
  }

  renderer.blitRGB565(bgPixels, 0, 0, bgMeta->width, bgMeta->height);
  captureCharacterFootprint(loader, cache, bgCache, character, node.spriteId);
}

void drawCharacterNode(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                       Compositor &compositor, BackgroundCache &bgCache, const SceneNode &node,
                       bool restoreBackground) {
  if (!node.visible || !node.spriteId || !node.spriteId[0]) {
    return;
  }
  if (restoreBackground) {
    bgCache.restore(renderer);
  }
  compositor.blitSprite(renderer, loader, cache, node.spriteId, node.x, node.y);
}

void drawHudNode(IRenderer &renderer, const SceneNode &node) {
  if (!node.visible || !node.text || !node.text[0]) {
    return;
  }
  renderer.fillRect(0, 0, renderer.width(), kHudBandHeight, kHudClearColor);
  renderer.drawText(node.x, node.y, node.text, kTextColor);
}

void drawSpeechBubbleNode(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                          const char *bgSpriteId, const SceneNode &node) {
  int bandY = renderer.height() - kBubbleBandHeight;
  if (node.text && node.text[0]) {
    renderer.fillRect(0, bandY, renderer.width(), kBubbleBandHeight, kHudClearColor);
    renderer.drawText(node.x, renderer.height() - 12, node.text, kTextColor);
    return;
  }

  if (!bgSpriteId || !bgSpriteId[0]) {
    renderer.fillRect(0, bandY, renderer.width(), kBubbleBandHeight, kHudClearColor);
    return;
  }

  const SpriteMeta *bgMeta = loader.findSprite(bgSpriteId);
  const uint16_t *bgPixels =
      bgMeta ? cache.get(bgSpriteId, loader, bgMeta) : nullptr;
  if (!bgMeta || !bgPixels || bandY >= bgMeta->height) {
    renderer.fillRect(0, bandY, renderer.width(), kBubbleBandHeight, kHudClearColor);
    return;
  }

  int copyH = kBubbleBandHeight;
  if (bandY + copyH > bgMeta->height) {
    copyH = bgMeta->height - bandY;
  }
  if (copyH > 0) {
    renderer.blitRGB565(bgPixels + bandY * bgMeta->width, 0, bandY, bgMeta->width, copyH);
  }
}

}  // namespace

void CharacterRenderer::drawScene(IRenderer &renderer, const Scene &scene, DirtyFlags dirty,
                                  PackLoader &loader, SpriteCache &cache, Compositor &compositor,
                                  BackgroundCache &bgCache) {
  if (dirty == DirtyFull) {
    drawBackgroundNode(renderer, loader, cache, compositor, bgCache, scene.background,
                     scene.character);
    drawCharacterNode(renderer, loader, cache, compositor, bgCache, scene.character, false);
    if (scene.hud.visible) {
      drawHudNode(renderer, scene.hud);
    }
    if (scene.speechBubble.visible) {
      drawSpeechBubbleNode(renderer, loader, cache, scene.background.spriteId, scene.speechBubble);
    }
    return;
  }

  if (scene.background.dirty) {
    drawBackgroundNode(renderer, loader, cache, compositor, bgCache, scene.background,
                     scene.character);
  }

  if (scene.character.dirty) {
    const bool restorePatch = !scene.background.dirty;
    drawCharacterNode(renderer, loader, cache, compositor, bgCache, scene.character, restorePatch);
  }

  if (scene.hud.dirty) {
    drawHudNode(renderer, scene.hud);
  }

  if (scene.speechBubble.dirty) {
    drawSpeechBubbleNode(renderer, loader, cache, scene.background.spriteId, scene.speechBubble);
  }
}
