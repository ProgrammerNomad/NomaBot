#include "scene_builder.h"

#include <cstring>

namespace {

const char *sceneIdFromBackground(const char *bgSpriteId) {
  if (!bgSpriteId || !bgSpriteId[0]) {
    return "dark";
  }
  if (strncmp(bgSpriteId, "bg_", 3) == 0) {
    return bgSpriteId + 3;
  }
  return bgSpriteId;
}

void markNodeDirty(SceneNode &node, bool dirtyFlag) {
  node.dirty = dirtyFlag && node.visible;
}

void applyDirtyFlags(Scene &scene, DirtyFlags dirty) {
  if (dirty == DirtyFull) {
    markNodeDirty(scene.background, true);
    markNodeDirty(scene.expression, true);
    return;
  }

  markNodeDirty(scene.background, hasDirty(dirty, DirtyBackground));
  markNodeDirty(scene.expression, hasDirty(dirty, DirtyCharacter));
  if (hasDirty(dirty, DirtyBackground) && scene.expression.visible) {
    scene.expression.dirty = true;
  }
}

}  // namespace

Scene SceneBuilder::build(const RenderState &state, PackLoader &loader, DirtyFlags dirty) {
  Scene scene;
  scene.ambientMode = state.ambientMode;

  const char *bgSprite = state.backgroundSpriteId;
  if (!bgSprite || !bgSprite[0]) {
    bgSprite = loader.defaultBackgroundSprite();
  }
  const char *bodySprite = state.bodySpriteId;
  if (!bodySprite || !bodySprite[0]) {
    bodySprite = "eyes_neutral";
  }

  scene.sceneId = sceneIdFromBackground(bgSprite);
  scene.background.id = scene.sceneId;
  scene.background.spriteId = bgSprite;
  scene.background.x = 0;
  scene.background.y = 0;
  scene.background.z = kSceneZBackground;
  scene.background.visible = bgSprite && bgSprite[0];

  scene.hud.visible = false;
  scene.speechBubble.visible = false;
  scene.ambientBar.visible = false;
  scene.character.visible = false;

  if (state.ambientMode == AmbientDisplayMode::ClockScreen) {
    scene.largeLine1 = state.clockText;
    scene.largeLine2 = state.clockDateText;
    scene.expression.visible = false;
    applyDirtyFlags(scene, dirty);
    scene.nodeCount = sceneVisibleNodeCount(scene);
    return scene;
  }

  if (state.ambientMode == AmbientDisplayMode::WeatherScreen) {
    scene.largeLine1 = state.weatherText;
    scene.largeLine2 = state.weatherConditionText;
    scene.largeLine3 = state.weatherCityText;
    scene.expression.visible = false;
    applyDirtyFlags(scene, dirty);
    scene.nodeCount = sceneVisibleNodeCount(scene);
    return scene;
  }

  scene.expression.id = bodySprite;
  scene.expression.spriteId = bodySprite;
  scene.expression.x = 0;
  scene.expression.y = 0;
  scene.expression.z = kSceneZExpression;
  scene.expression.visible = bodySprite && bodySprite[0];

  scene.nodeCount = sceneVisibleNodeCount(scene);
  applyDirtyFlags(scene, dirty);
  return scene;
}
