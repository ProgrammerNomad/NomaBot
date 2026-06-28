#pragma once

#include "assets/pack_loader.h"
#include "render/scene.h"
#include "render/render_state.h"

class SceneBuilder {
public:
  static Scene build(const RenderState &state, PackLoader &loader, DirtyFlags dirty);
};
