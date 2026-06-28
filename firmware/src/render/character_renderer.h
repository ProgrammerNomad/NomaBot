#pragma once

#include "animation/compositor.h"
#include "assets/pack_loader.h"
#include "assets/sprite_cache.h"
#include "render/background_cache.h"
#include "render/scene.h"
#include "render/render_state.h"
#include "renderer/renderer.hpp"

class CharacterRenderer {
public:
  static void drawScene(IRenderer &renderer, const Scene &scene, DirtyFlags dirty,
                        PackLoader &loader, SpriteCache &cache, Compositor &compositor,
                        BackgroundCache &bgCache);
};
