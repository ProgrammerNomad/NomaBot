#include "text_scene_renderer.h"

#include <stdio.h>
#include <string.h>

namespace {

constexpr int kBandHeaderY = 0;
constexpr int kBandHeaderH = 18;
constexpr int kBandMetaY = 18;
constexpr int kBandMetaH = 18;
constexpr int kBandEnergyY = 34;
constexpr int kBandEnergyH = 18;
constexpr int kBandBehaviorY = 50;
constexpr int kBandBehaviorH = 18;
constexpr int kBandOverlayH = 28;

} // namespace

void TextSceneRenderer::clearBand(IRenderer &renderer, int y, int h) {
  renderer.fillRect(0, y, renderer.width(), h, 0x0000);
}

void TextSceneRenderer::drawHeader(IRenderer &renderer, const RenderState &state) {
  clearBand(renderer, kBandHeaderY, kBandHeaderH);
  char header[48];
  snprintf(header, sizeof(header), "%s · %s", state.lifeMode ? state.lifeMode : "work",
           state.activity ? state.activity : "idle");
  renderer.drawText(4, 8, header, 0xFFFF);
}

void TextSceneRenderer::drawMeta(IRenderer &renderer, const RenderState &state) {
  clearBand(renderer, kBandMetaY, kBandMetaH);
  char meta[48];
  if (state.goal && state.goal[0] && strcmp(state.goal, "none") != 0) {
    snprintf(meta, sizeof(meta), "%s · %s · %d%%", state.emotion ? state.emotion : "neutral",
             state.goal, state.goalProgress);
    renderer.drawText(4, 24, meta, 0xAD55);
  } else if (state.emotion && state.emotion[0] && strcmp(state.emotion, "neutral") != 0) {
    snprintf(meta, sizeof(meta), "%s", state.emotion);
    renderer.drawText(4, 24, meta, 0xAD55);
  }
}

void TextSceneRenderer::drawEnergy(IRenderer &renderer, const RenderState &state) {
  clearBand(renderer, kBandEnergyY, kBandEnergyH);
  char energyLine[24];
  snprintf(energyLine, sizeof(energyLine), "Energy: %d", state.displayEnergy);
  renderer.drawText(4, 40, energyLine, 0x7BEF);
}

void TextSceneRenderer::drawBehavior(IRenderer &renderer, const RenderState &state) {
  clearBand(renderer, kBandBehaviorY, kBandBehaviorH);
  if (state.curiosity) {
    renderer.drawText(4, 56, "I wonder...", 0xFD20);
  } else if (state.behaviorLabel && state.behaviorLabel[0]) {
    renderer.drawText(4, 56, state.behaviorLabel, 0xFFFF);
  }
}

void TextSceneRenderer::drawOverlay(IRenderer &renderer, const RenderState &state) {
  int y = renderer.height() - kBandOverlayH;
  clearBand(renderer, y, kBandOverlayH);
  if (state.overlayText && state.overlayText[0]) {
    renderer.drawText(4, renderer.height() - 20, state.overlayText, 0xFFFF);
  }
}

void TextSceneRenderer::drawAll(IRenderer &renderer, const RenderState &state) {
  drawHeader(renderer, state);
  drawMeta(renderer, state);
  drawEnergy(renderer, state);
  drawBehavior(renderer, state);
  drawOverlay(renderer, state);
}
