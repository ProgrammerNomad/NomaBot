#pragma once

#include <string>

#include "accessory_manager.h"
#include "animation/animation_graph.h"
#include "animation/clip_player.h"
#include "animation/compositor.h"
#include "assets/asset_registry.h"
#include "assets/pack_loader.h"
#include "assets/sprite_cache.h"
#include "behavior/behavior_engine.h"
#include "render/render_mode.h"
#include "render/text_scene_renderer.h"
#include "renderer/renderer.hpp"

enum class CharacterLoadError {
  None,
  Pack,
  Graph,
};

const char *characterLoadErrorLabel(CharacterLoadError err);

class CharacterRuntime {
public:
  void begin(IRenderer *renderer);
  bool loadCharacter(PackLoader &loader, const char *characterId);
  void unload();
  void useBehaviorDefaults();

  void tick(unsigned long nowMs);
  void render();

  void setLifeMode(const char *mode);
  void setActivity(const char *activity);
  void setEmotion(const char *emotion);

  void playAnimation(const char *animationId);
  void setState(const char *state);
  void setMessage(const char *text);
  void setBackground(const char *backgroundKey);

  void setRenderMode(RenderMode mode) { _renderMode = mode; }
  RenderMode renderMode() const { return _renderMode; }
  bool packLoaded() const { return _loader != nullptr; }
  bool textModeActive() const;

  CharacterLoadError lastLoadError() const { return _lastLoadError; }
  const PackInfo *packInfo() const;
  const char *characterId() const { return _characterId.c_str(); }
  const char *currentAnimation() const;
  const char *currentState() const { return _behavior.activity(); }
  const char *lifeMode() const { return _behavior.lifeMode(); }
  const char *currentActivity() const { return _behavior.activity(); }
  const char *currentEmotion() const { return _behavior.emotion(); }
  const char *currentBehavior() const { return _behavior.behaviorId(); }
  const char *nextBehavior() const { return _behavior.nextBehaviorId(); }
  int timeInBehaviorSec(unsigned long nowMs) const { return _behavior.timeInBehaviorSec(nowMs); }
  int currentFrame() const { return _clipPlayer.currentFrameIndex(); }
  int fps() const { return _fps; }
  void updateFps(unsigned long nowMs);

  BehaviorEngine &behaviorEngine() { return _behavior; }

private:
  IRenderer *_renderer = nullptr;
  PackLoader *_loader = nullptr;
  AssetRegistry _assets;
  SpriteCache _cache;
  AnimationGraph _graph;
  ClipPlayer _clipPlayer;
  AccessoryManager _accessories;
  Compositor _compositor;
  TextSceneRenderer _textRenderer;
  BehaviorEngine _behavior;

  CharacterLoadError _lastLoadError = CharacterLoadError::None;
  RenderMode _renderMode = RenderMode::Text;
  bool _overrideAnimation = false;
  std::string _characterId = "nomabot";
  std::string _backgroundSprite;
  std::string _message;
  std::string _activeClipId;
  unsigned long _lastFpsMs = 0;
  int _frameCount = 0;
  int _fps = 0;

  void applyClip(const char *animationId);
  void syncClipFromBehavior();
};
