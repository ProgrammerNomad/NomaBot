#pragma once

#include "animation/compositor.h"
#include "assets/asset_registry.h"
#include "assets/pack_loader.h"
#include "assets/sprite_cache.h"
#include "render/render_state.h"
#include "render/text_scene_renderer.h"
#include "renderer/renderer.hpp"

class RenderScheduler {
public:
  void begin(IRenderer *renderer);
  void setSpriteContext(PackLoader *loader, SpriteCache *cache, AssetRegistry *assets,
                        Compositor *compositor);
  void render(const RenderState &state, DirtyFlags dirty, bool textMode);

private:
  IRenderer *_renderer = nullptr;
  PackLoader *_loader = nullptr;
  SpriteCache *_cache = nullptr;
  AssetRegistry *_assets = nullptr;
  Compositor *_compositor = nullptr;

  void drawTextLayers(const RenderState &state, DirtyFlags dirty);
  void drawSpriteLayers(const RenderState &state, DirtyFlags dirty);
};
