#pragma once

#include "render/render_state.h"
#include "renderer/renderer.hpp"

class TextSceneRenderer {
public:
  static void clearBand(IRenderer &renderer, int y, int h);
  static void drawHeader(IRenderer &renderer, const RenderState &state);
  static void drawMeta(IRenderer &renderer, const RenderState &state);
  static void drawEnergy(IRenderer &renderer, const RenderState &state);
  static void drawBehavior(IRenderer &renderer, const RenderState &state);
  static void drawOverlay(IRenderer &renderer, const RenderState &state);
  static void drawAll(IRenderer &renderer, const RenderState &state);
};
