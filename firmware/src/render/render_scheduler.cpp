#include "render_scheduler.h"

void RenderScheduler::begin(IRenderer *renderer) { _renderer = renderer; }

void RenderScheduler::setSpriteContext(PackLoader *loader, SpriteCache *cache,
                                       AssetRegistry *assets, Compositor *compositor) {
  _loader = loader;
  _cache = cache;
  _assets = assets;
  _compositor = compositor;
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
  if (!_renderer || !_loader || !_cache || !_assets || !_compositor) {
    return;
  }

  if (dirty == DirtyFull || hasDirty(dirty, DirtyBackground) || hasDirty(dirty, DirtyCharacter) ||
      hasDirty(dirty, DirtyBehavior) || hasDirty(dirty, DirtyMessage)) {
    const char *body = state.bodySpriteId ? state.bodySpriteId : "body_idle_01";
    const char *bg = state.backgroundSpriteId ? state.backgroundSpriteId : "";
    const char *label = state.behaviorLabel ? state.behaviorLabel : "";
    const char *overlay = state.overlayText ? state.overlayText : "";
    _compositor->render(*_renderer, *_loader, *_cache, *_assets, bg, body, _loader->anchorX(),
                        _loader->anchorY(), overlay, label);
  }
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
