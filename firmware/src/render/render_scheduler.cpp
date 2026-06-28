#include "render_scheduler.h"

void RenderScheduler::begin(IRenderer *renderer) { _renderer = renderer; }

void RenderScheduler::setSpriteContext(PackLoader *loader, SpriteCache *cache,
                                       AssetRegistry *assets, Compositor *compositor) {
  _loader = loader;
  _cache = cache;
  _assets = assets;
  _compositor = compositor;
  if (!loader) {
    _bgCache.reset();
  }
}

void RenderScheduler::drawTextLayers(const RenderState &state, DirtyFlags dirty) {
  if (!_renderer) {
    return;
  }

  if (dirty == DirtyFull) {
    _renderer->fillScreen(0x0000);
    TextSceneRenderer::drawAll(*_renderer, state);
    return;
  }

  if (hasDirty(dirty, DirtyHeader)) {
    TextSceneRenderer::drawHeader(*_renderer, state);
  }
  if (hasDirty(dirty, DirtyMeta)) {
    TextSceneRenderer::drawMeta(*_renderer, state);
  }
  if (hasDirty(dirty, DirtyEnergy)) {
    TextSceneRenderer::drawEnergy(*_renderer, state);
  }
  if (hasDirty(dirty, DirtyBehavior)) {
    TextSceneRenderer::drawBehavior(*_renderer, state);
  }
  if (hasDirty(dirty, DirtyMessage)) {
    TextSceneRenderer::drawOverlay(*_renderer, state);
  }
}

void RenderScheduler::drawSpriteLayers(const RenderState &state, DirtyFlags dirty) {
  if (!_renderer || !_loader || !_cache || !_compositor) {
    return;
  }

  _lastScene = SceneBuilder::build(state, *_loader, dirty);
  CharacterRenderer::drawScene(*_renderer, _lastScene, dirty, *_loader, *_cache, *_compositor,
                               _bgCache);
}

void RenderScheduler::render(const RenderState &state, DirtyFlags dirty, bool textMode) {
  if (!anyDirty(dirty) || !_renderer) {
    return;
  }

  if (textMode) {
    drawTextLayers(state, dirty);
  } else {
    drawSpriteLayers(state, dirty);
  }
}
