#pragma once

#include "render_state.h"

static constexpr int kSceneMaxNodes = 4;

static constexpr int kSceneZBackground = 0;
static constexpr int kSceneZCharacter = 10;
static constexpr int kSceneZProp = 20;  // reserved M6+
static constexpr int kSceneZHud = 30;
static constexpr int kSceneZSpeechBubble = 40;

struct SceneNode {
  const char *id = nullptr;
  const char *spriteId = nullptr;
  const char *text = nullptr;
  int x = 0;
  int y = 0;
  int z = 0;
  bool visible = false;
  bool dirty = false;
};

struct Scene {
  const char *sceneId = "office";
  SceneNode background;
  SceneNode character;
  SceneNode hud;
  SceneNode speechBubble;
  int nodeCount = 0;
};

struct SceneDiagnostics {
  const char *scene = "office";
  const char *body = "";
  const char *eyes = "";
  const char *overlay = "";
  int renderObjects = 0;
};

inline int sceneVisibleNodeCount(const Scene &scene) {
  int count = 0;
  if (scene.background.visible) {
    count++;
  }
  if (scene.character.visible) {
    count++;
  }
  if (scene.hud.visible) {
    count++;
  }
  if (scene.speechBubble.visible) {
    count++;
  }
  return count;
}

inline SceneDiagnostics sceneToDiagnostics(const Scene &scene) {
  SceneDiagnostics diag;
  diag.scene = scene.sceneId ? scene.sceneId : "office";
  diag.body = scene.character.visible && scene.character.id ? scene.character.id : "";
  diag.eyes = "";
  diag.overlay =
      scene.speechBubble.visible && scene.speechBubble.text ? scene.speechBubble.text : "";
  diag.renderObjects = sceneVisibleNodeCount(scene);
  return diag;
}
