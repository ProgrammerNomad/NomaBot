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

void RenderScheduler::render(const RenderState &state, DirtyFlags dirty) {
  if (!anyDirty(dirty) || !_renderer || !_loader || !_cache || !_compositor) {
    return;
  }

  _lastScene = SceneBuilder::build(state, *_loader, dirty);
  CharacterRenderer::drawScene(*_renderer, _lastScene, dirty, *_loader, *_cache, *_compositor,
                               _bgCache);
}
