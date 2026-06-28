#pragma once

#include <string>

#include "accessory_manager.h"
#include "animation/animation_graph.h"
#include "animation/clip_player.h"
#include "animation/compositor.h"
#include "assets/asset_registry.h"
#include "assets/pack_loader.h"
#include "assets/sprite_cache.h"
#include "brain/brain.h"
#include "render/dirty_tracker.h"
#include "render/overlay_manager.h"
#include "render/render_mode.h"
#include "render/render_scheduler.h"
#include "render/render_state.h"
#include "render/scene.h"
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
  RenderState buildRenderState() const;
  DirtyFlags collectDirtyFlags();
  void render(DirtyFlags dirty);
  void present();
  void invalidateRender(DirtyFlags flags = DirtyFull);

  void setLifeMode(const char *mode);
  void setActivity(const char *activity);
  void applyActivityCommand(const char *activity, const char *source = "protocol");
  void setEmotion(const char *emotion);
  void setSeason(const char *season);
  void triggerHabit(const char *habitId);

  void noteCommandSource(const char *source);
  const char *lastCommandSource() const { return _lastCommandSource.c_str(); }
  unsigned long renderCount() const { return _renderCount; }
  unsigned long lastRenderMs() const { return _lastRenderMs; }
  unsigned long lastBrainTickMs() const { return _lastBrainTickMs; }
  int overlayQueueDepth() const { return _overlays.queueDepth(); }
  DirtyFlags lastDirtyFlags() const { return _lastDirtyFlags; }
  SceneDiagnostics lastSceneDiagnostics() const { return _scheduler.lastSceneDiagnostics(); }
  const char *bodySpriteId() const {
    return _bodySpriteId.empty() ? nullptr : _bodySpriteId.c_str();
  }

  void playAnimation(const char *animationId);
  void setState(const char *state);
  void setMessage(const char *id, const char *text, int priority, unsigned long durationMs = 5000);
  void setBackground(const char *backgroundKey);

  void setRenderMode(RenderMode mode) { _renderMode = mode; }
  RenderMode renderMode() const { return _renderMode; }
  bool packLoaded() const { return _loader != nullptr; }
  bool textModeActive() const;

  CharacterLoadError lastLoadError() const { return _lastLoadError; }
  const PackInfo *packInfo() const;
  const char *characterId() const { return _characterId.c_str(); }
  const char *currentAnimation() const;
  const char *lifeMode() const { return _brain.lifeMode(); }
  const char *currentActivity() const { return _brain.activity(); }
  const char *currentEmotion() const { return _brain.emotion(); }
  const char *currentBehavior() const { return _brain.behaviorId(); }
  const char *nextBehavior() const { return _brain.nextBehaviorId(); }
  const char *goal() const { return _brain.goal(); }
  int goalProgress() const { return _brain.goalProgress(); }
  int energy() const { return _brain.energy(); }
  int boredom() const { return _brain.boredom(); }
  int timeInBehaviorSec(unsigned long nowMs) const { return _brain.timeInBehaviorSec(nowMs); }
  int lastCoffeeMinAgo(unsigned long nowMs) const { return _brain.lastCoffeeMinAgo(nowMs); }
  int currentFrame() const { return _clipPlayer.currentFrameIndex(); }
  int fps() const { return _fps; }
  void updateFps(unsigned long nowMs);

  Brain &brain() { return _brain; }

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
  OverlayManager _overlays;
  DirtyTracker _dirtyTracker;
  RenderScheduler _scheduler;
  Brain _brain;

  CharacterLoadError _lastLoadError = CharacterLoadError::None;
  RenderMode _renderMode = RenderMode::Text;
  bool _overrideAnimation = false;
  std::string _characterId = "nomabot";
  std::string _backgroundSprite;
  std::string _activeClipId;
  std::string _bodySpriteId;
  unsigned long _lastFpsMs = 0;
  int _frameCount = 0;
  int _fps = 0;
  int _lastClipFrame = -1;
  std::string _lastCommandSource = "brain";
  unsigned long _renderCount = 0;
  unsigned long _lastRenderMs = 0;
  unsigned long _lastBrainTickMs = 0;
  DirtyFlags _lastDirtyFlags = DirtyNone;

  void applyClip(const char *animationId);
  void syncClipFromBehavior();
  void syncSpriteContext();
};
