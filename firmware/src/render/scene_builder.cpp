#include "scene_builder.h"

#include <cstring>

namespace {

const char *sceneIdFromBackground(const char *bgSpriteId) {
  if (!bgSpriteId || !bgSpriteId[0]) {
    return "office";
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
    markNodeDirty(scene.ambientBar, true);
    markNodeDirty(scene.character, true);
    markNodeDirty(scene.expression, true);
    markNodeDirty(scene.hud, true);
    markNodeDirty(scene.speechBubble, true);
    return;
  }

  markNodeDirty(scene.background, hasDirty(dirty, DirtyBackground));
  markNodeDirty(scene.ambientBar, hasDirty(dirty, DirtyBehavior));
  markNodeDirty(scene.character, hasDirty(dirty, DirtyCharacter));
  markNodeDirty(scene.expression, hasDirty(dirty, DirtyCharacter));
  markNodeDirty(scene.hud, hasDirty(dirty, DirtyBehavior));
  markNodeDirty(scene.speechBubble, hasDirty(dirty, DirtyMessage));
  if (hasDirty(dirty, DirtyMessage)) {
    scene.speechBubble.dirty = true;
  }

  if (hasDirty(dirty, DirtyBackground)) {
    if (scene.character.visible) {
      scene.character.dirty = true;
    }
    if (scene.expression.visible) {
      scene.expression.dirty = true;
    }
  }
  if (hasDirty(dirty, DirtyCharacter) && scene.expression.visible) {
    scene.expression.dirty = true;
  }
}

}  // namespace

Scene SceneBuilder::build(const RenderState &state, PackLoader &loader, DirtyFlags dirty) {
  Scene scene;
  const bool eyesOnly = loader.eyesOnlyMode();

  const char *bgSprite = state.backgroundSpriteId;
  if (!bgSprite || !bgSprite[0]) {
    bgSprite = loader.defaultBackgroundSprite();
  }
  const char *bodySprite = state.bodySpriteId;
  if (!bodySprite || !bodySprite[0]) {
    bodySprite = eyesOnly ? "eyes_neutral" : "body_idle_01";
  }

  scene.sceneId = sceneIdFromBackground(bgSprite);

  scene.background.id = scene.sceneId;
  scene.background.spriteId = bgSprite;
  scene.background.x = 0;
  scene.background.y = 0;
  scene.background.z = kSceneZBackground;
  scene.background.visible = bgSprite && bgSprite[0];

  const char *clockText = state.clockText ? state.clockText : "";
  const char *weatherText = state.weatherText ? state.weatherText : "";
  scene.ambientBar.id = clockText;
  scene.ambientBar.spriteId = state.weatherIcon ? state.weatherIcon : "";
  scene.ambientBar.text = weatherText;
  scene.ambientBar.x = 4;
  scene.ambientBar.y = 6;
  scene.ambientBar.z = kSceneZAmbient;
  scene.ambientBar.visible = eyesOnly && (clockText[0] != '\0' || weatherText[0] != '\0');

  if (eyesOnly) {
    scene.character.visible = false;
    scene.expression.id = bodySprite;
    scene.expression.spriteId = bodySprite;
    scene.expression.x = 0;
    scene.expression.y = 0;
    scene.expression.z = kSceneZExpression;
    scene.expression.visible = bodySprite && bodySprite[0];
  } else {
    scene.character.id = bodySprite;
    scene.character.spriteId = bodySprite;
    scene.character.x = loader.anchorX();
    scene.character.y = loader.anchorY();
    scene.character.z = kSceneZCharacter;
    scene.character.visible = bodySprite && bodySprite[0];

    const char *emotion = state.emotion ? state.emotion : "neutral";
    const char *faceSprite = loader.expressionForEmotion(emotion);
    scene.expression.id = faceSprite;
    scene.expression.spriteId = faceSprite;
    scene.expression.x = loader.expressionAnchorX();
    scene.expression.y = loader.expressionAnchorY();
    scene.expression.z = kSceneZExpression;
    scene.expression.visible = faceSprite && faceSprite[0];
  }

  const char *label = state.behaviorLabel ? state.behaviorLabel : "";
  scene.hud.id = "hud";
  scene.hud.text = label;
  scene.hud.x = 4;
  scene.hud.y = 8;
  scene.hud.z = kSceneZHud;
  scene.hud.visible = label[0] != '\0' && loader.hudEnabled();

  const char *overlay = state.overlayText ? state.overlayText : "";
  scene.speechBubble.id = overlay[0] ? "overlay" : "speech_bubble";
  scene.speechBubble.text = overlay;
  scene.speechBubble.x = 4;
  scene.speechBubble.y = 0;
  scene.speechBubble.z = kSceneZSpeechBubble;
  scene.speechBubble.visible = overlay[0] != '\0';

  scene.nodeCount = sceneVisibleNodeCount(scene);
  applyDirtyFlags(scene, dirty);
  return scene;
}
