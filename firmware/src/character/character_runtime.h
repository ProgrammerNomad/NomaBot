#pragma once

#include <string>

#include "accessory_manager.h"
#include "animation/animation_graph.h"
#include "animation/clip_player.h"
#include "animation/compositor.h"
#include "assets/asset_registry.h"
#include "assets/pack_loader.h"
#include "assets/sprite_cache.h"
#include "renderer/renderer.hpp"

class CharacterRuntime {
public:
  void begin(IRenderer *renderer);
  bool loadCharacter(PackLoader &loader, const char *characterId);
  void unload();

  void tick(unsigned long nowMs);
  void render();

  void playAnimation(const char *animationId);
  void setState(const char *state);
  void setMessage(const char *text);
  void setBackground(const char *backgroundKey);

  const PackInfo *packInfo() const;
  const char *characterId() const { return _characterId.c_str(); }
  const char *currentAnimation() const { return _clipPlayer.clipId(); }
  const char *currentState() const { return _graph.currentState(); }
  int currentFrame() const { return _clipPlayer.currentFrameIndex(); }
  int fps() const { return _fps; }
  void updateFps(unsigned long nowMs);

private:
  IRenderer *_renderer = nullptr;
  PackLoader *_loader = nullptr;
  AssetRegistry _assets;
  SpriteCache _cache;
  AnimationGraph _graph;
  ClipPlayer _clipPlayer;
  AccessoryManager _accessories;
  Compositor _compositor;

  std::string _characterId;
  std::string _backgroundSprite;
  std::string _message;
  std::string _activeClipId;
  unsigned long _lastFpsMs = 0;
  int _frameCount = 0;
  int _fps = 0;

  void applyClip(const char *animationId);
};
