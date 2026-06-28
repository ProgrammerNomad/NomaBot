#include "character_renderer.h"

namespace {

constexpr int kHudBandHeight = 18;
constexpr uint16_t kHudClearColor = 0x0000;
constexpr uint16_t kTextColor = 0xFFFF;
constexpr uint16_t kBubbleFill = 0xEF5D;
constexpr uint16_t kBubbleBorder = 0x4A49;
constexpr int kBubblePadX = 8;
constexpr int kBubblePadY = 6;
constexpr int kBubbleRadius = 6;

int textWidthEstimate(const char *text) {
  if (!text) {
    return 0;
  }
  int len = 0;
  while (text[len]) {
    len++;
  }
  return len * 6;
}

void fillRoundedRect(IRenderer &renderer, int x, int y, int w, int h, int r, uint16_t color) {
  if (w <= 0 || h <= 0) {
    return;
  }
  if (r < 1) {
    renderer.fillRect(x, y, w, h, color);
    return;
  }
  renderer.fillRect(x + r, y, w - 2 * r, h, color);
  renderer.fillRect(x, y + r, w, h - 2 * r, color);
  renderer.fillRect(x, y, r, r, color);
  renderer.fillRect(x + w - r, y, r, r, color);
  renderer.fillRect(x, y + h - r, r, r, color);
  renderer.fillRect(x + w - r, y + h - r, r, r, color);
}

void strokeRoundedRect(IRenderer &renderer, int x, int y, int w, int h, int r, uint16_t color) {
  renderer.fillRect(x + r, y, w - 2 * r, 1, color);
  renderer.fillRect(x + r, y + h - 1, w - 2 * r, 1, color);
  renderer.fillRect(x, y + r, 1, h - 2 * r, color);
  renderer.fillRect(x + w - 1, y + r, 1, h - 2 * r, color);
}

void drawBubbleTail(IRenderer &renderer, int centerX, int topY) {
  renderer.fillRect(centerX - 1, topY, 3, 4, kBubbleFill);
  renderer.fillRect(centerX - 2, topY + 4, 5, 2, kBubbleBorder);
}

bool captureFootprint(PackLoader &loader, SpriteCache &cache, BackgroundCache &bgCache,
                      const char *bgSpriteId, int bodyAnchorX, int bodyAnchorY,
                      const char *bodySpriteId, const char *faceSpriteId, int exprAnchorX,
                      int exprAnchorY) {
  if (!bgSpriteId || !bgSpriteId[0] || !bodySpriteId || !bodySpriteId[0]) {
    bgCache.reset();
    return false;
  }

  const SpriteMeta *bgMeta = loader.findSprite(bgSpriteId);
  const SpriteMeta *bodyMeta = loader.findSprite(bodySpriteId);
  if (!bgMeta || !bodyMeta) {
    bgCache.reset();
    return false;
  }

  const uint16_t *bgPixels = cache.get(bgSpriteId, loader, bgMeta);
  if (!bgPixels) {
    bgCache.reset();
    return false;
  }

  int drawX = bodyAnchorX - bodyMeta->width / 2;
  int drawY = bodyAnchorY;
  int patchW = bodyMeta->width;
  int patchH = bodyMeta->height;

  if (faceSpriteId && faceSpriteId[0]) {
    const SpriteMeta *faceMeta = loader.findSprite(faceSpriteId);
    if (faceMeta) {
      int fx = exprAnchorX - faceMeta->width / 2;
      int fy = exprAnchorY;
      int minX = drawX < fx ? drawX : fx;
      int minY = drawY < fy ? drawY : fy;
      int maxX = (drawX + patchW) > (fx + faceMeta->width) ? (drawX + patchW) : (fx + faceMeta->width);
      int maxY =
          (drawY + patchH) > (fy + faceMeta->height) ? (drawY + patchH) : (fy + faceMeta->height);
      drawX = minX;
      drawY = minY;
      patchW = maxX - minX;
      patchH = maxY - minY;
    }
  }

  return bgCache.captureFromBackground(bgPixels, bgMeta->width, bgMeta->height, drawX, drawY, patchW,
                                       patchH);
}

void drawBackgroundNode(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                        Compositor &compositor, BackgroundCache &bgCache, const SceneNode &node,
                        const Scene &scene) {
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
  captureFootprint(loader, cache, bgCache, node.spriteId, scene.character.x, scene.character.y,
                   scene.character.spriteId, scene.expression.spriteId, scene.expression.x,
                   scene.expression.y);
}

void drawCharacterNode(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                       Compositor &compositor, const SceneNode &node) {
  if (!node.visible || !node.spriteId || !node.spriteId[0]) {
    return;
  }
  compositor.blitSprite(renderer, loader, cache, node.spriteId, node.x, node.y, false);
}

void drawExpressionNode(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                        Compositor &compositor, const SceneNode &node) {
  if (!node.visible || !node.spriteId || !node.spriteId[0]) {
    return;
  }
  compositor.blitSprite(renderer, loader, cache, node.spriteId, node.x, node.y, true);
}

void drawHudNode(IRenderer &renderer, const SceneNode &node) {
  if (!node.visible || !node.text || !node.text[0]) {
    return;
  }
  renderer.fillRect(0, 0, renderer.width(), kHudBandHeight, kHudClearColor);
  renderer.drawText(node.x, node.y, node.text, kTextColor);
}

void restoreBubbleBand(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                       const char *bgSpriteId, int bandY, int bandH) {
  if (!bgSpriteId || !bgSpriteId[0]) {
    renderer.fillRect(0, bandY, renderer.width(), bandH, kHudClearColor);
    return;
  }
  const SpriteMeta *bgMeta = loader.findSprite(bgSpriteId);
  const uint16_t *bgPixels = bgMeta ? cache.get(bgSpriteId, loader, bgMeta) : nullptr;
  if (!bgMeta || !bgPixels || bandY >= bgMeta->height) {
    renderer.fillRect(0, bandY, renderer.width(), bandH, kHudClearColor);
    return;
  }
  int copyH = bandH;
  if (bandY + copyH > bgMeta->height) {
    copyH = bgMeta->height - bandY;
  }
  if (copyH > 0) {
    renderer.blitRGB565(bgPixels + bandY * bgMeta->width, 0, bandY, bgMeta->width, copyH);
  }
}

void drawSpeechBubbleNode(IRenderer &renderer, PackLoader &loader, SpriteCache &cache,
                          const char *bgSpriteId, const SceneNode &node) {
  int bandY = renderer.height() - 56;
  int bandH = 56;
  if (!node.text || !node.text[0]) {
    restoreBubbleBand(renderer, loader, cache, bgSpriteId, bandY, bandH);
    return;
  }

  restoreBubbleBand(renderer, loader, cache, bgSpriteId, bandY, bandH);

  int tw = textWidthEstimate(node.text);
  if (tw > renderer.width() - 24) {
    tw = renderer.width() - 24;
  }
  int bw = tw + kBubblePadX * 2;
  int bh = 22 + kBubblePadY * 2;
  int bx = node.x;
  if (bx + bw > renderer.width() - 4) {
    bx = renderer.width() - bw - 4;
  }
  if (bx < 4) {
    bx = 4;
  }
  int by = renderer.height() - bh - 16;

  fillRoundedRect(renderer, bx, by, bw, bh, kBubbleRadius, kBubbleFill);
  strokeRoundedRect(renderer, bx, by, bw, bh, kBubbleRadius, kBubbleBorder);
  drawBubbleTail(renderer, bx + bw / 2, by + bh);
  renderer.drawText(bx + kBubblePadX, by + kBubblePadY + 8, node.text, kBubbleBorder);
}

}  // namespace

void CharacterRenderer::drawScene(IRenderer &renderer, const Scene &scene, DirtyFlags dirty,
                                  PackLoader &loader, SpriteCache &cache, Compositor &compositor,
                                  BackgroundCache &bgCache) {
  if (dirty == DirtyFull) {
    drawBackgroundNode(renderer, loader, cache, compositor, bgCache, scene.background, scene);
    drawCharacterNode(renderer, loader, cache, compositor, scene.character);
    drawExpressionNode(renderer, loader, cache, compositor, scene.expression);
    if (scene.hud.visible) {
      drawHudNode(renderer, scene.hud);
    }
    if (scene.speechBubble.visible) {
      drawSpeechBubbleNode(renderer, loader, cache, scene.background.spriteId, scene.speechBubble);
    }
    return;
  }

  if (scene.background.dirty) {
    drawBackgroundNode(renderer, loader, cache, compositor, bgCache, scene.background, scene);
  }

  if (scene.character.dirty || scene.expression.dirty) {
    if (!scene.background.dirty) {
      bgCache.restore(renderer);
    }
    if (scene.character.dirty) {
      drawCharacterNode(renderer, loader, cache, compositor, scene.character);
    }
    if (scene.expression.dirty) {
      drawExpressionNode(renderer, loader, cache, compositor, scene.expression);
    }
  }

  if (scene.hud.dirty) {
    drawHudNode(renderer, scene.hud);
  }

  if (scene.speechBubble.dirty) {
    drawSpeechBubbleNode(renderer, loader, cache, scene.background.spriteId, scene.speechBubble);
  }
}
