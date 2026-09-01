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

  void tick(unsigned long nowMs);
  void present();
  void invalidateRender(DirtyFlags flags = DirtyFull);

  void setActivity(const char *activity);
  void setWeatherDisplay(const char *icon, const char *tempLine, const char *conditionLine,
                         const char *city);
  void setClock(const char *timeText, const char *dateText = nullptr);
  void setDisplayMode(AmbientDisplayMode mode);
  AmbientDisplayMode displayMode() const { return _ambientMode; }

  RenderMode renderMode() const { return _renderMode; }
  bool packLoaded() const { return _loader != nullptr; }

  CharacterLoadError lastLoadError() const { return _lastLoadError; }
  const PackInfo *packInfo() const;
  const char *characterId() const { return _characterId.c_str(); }
  const char *currentAnimation() const;
  const char *currentActivity() const { return _brain.activity(); }
  const char *currentBehavior() const { return _brain.behaviorId(); }
  const char *nextBehavior() const { return _brain.nextBehaviorId(); }
  int timeInBehaviorSec(unsigned long nowMs) const { return _brain.timeInBehaviorSec(nowMs); }
  int currentFrame() const { return _clipPlayer.currentFrameIndex(); }
  int fps() const { return _fps; }

  unsigned long renderCount() const { return _renderCount; }
  unsigned long lastRenderMs() const { return _lastRenderMs; }
  unsigned long lastBrainTickMs() const { return _lastBrainTickMs; }
  DirtyFlags lastDirtyFlags() const { return _lastDirtyFlags; }
  SceneDiagnostics lastSceneDiagnostics() const { return _scheduler.lastSceneDiagnostics(); }
  const char *bodySpriteId() const {
    return _bodySpriteId.empty() ? nullptr : _bodySpriteId.c_str();
  }

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
  OverlayManager _overlays;
  DirtyTracker _dirtyTracker;
  RenderScheduler _scheduler;
  Brain _brain;

  CharacterLoadError _lastLoadError = CharacterLoadError::None;
  RenderMode _renderMode = RenderMode::Eyes;
  bool _overrideAnimation = false;
  std::string _characterId = "eyes";
  std::string _backgroundSprite;
  std::string _activeClipId;
  std::string _bodySpriteId;
  std::string _clockText;
  std::string _clockDateText;
  std::string _weatherText;
  std::string _weatherConditionText;
  std::string _weatherCityText;
  std::string _weatherIcon;
  AmbientDisplayMode _ambientMode = AmbientDisplayMode::EyesAnim;
  unsigned long _lastFpsMs = 0;
  int _frameCount = 0;
  int _fps = 0;
  int _lastClipFrame = -1;
  unsigned long _renderCount = 0;
  unsigned long _lastRenderMs = 0;
  unsigned long _lastBrainTickMs = 0;
  DirtyFlags _lastDirtyFlags = DirtyNone;

  void applyClip(const char *animationId);
  void syncClipFromBehavior();
  void syncSpriteContext();
  void updateFps(unsigned long nowMs);
  RenderState buildRenderState() const;
  DirtyFlags collectDirtyFlags();
  void render(DirtyFlags dirty);
};
