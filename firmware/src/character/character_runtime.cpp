#include "character_runtime.h"

#include <Arduino.h>

void CharacterRuntime::begin(IRenderer *renderer) {
  _renderer = renderer;
}

bool CharacterRuntime::loadCharacter(PackLoader &loader, const char *characterId) {
  unload();
  if (!loader.load(characterId)) {
    return false;
  }
  _loader = &loader;
  _assets.bind(&loader);
  _characterId = characterId;

  if (!_graph.loadFromPack(loader.rootPath())) {
    Serial.println("Animation graph load failed");
    return false;
  }

  _backgroundSprite = loader.defaultBackgroundSprite();
  applyClip(_graph.currentAnimation());
  return true;
}

void CharacterRuntime::unload() {
  _cache.clear();
  _characterId.clear();
  _message.clear();
  _activeClipId.clear();
  _loader = nullptr;
  _assets.bind(nullptr);
  _accessories.clear();
}

const PackInfo *CharacterRuntime::packInfo() const {
  return _loader ? &_loader->info() : nullptr;
}

void CharacterRuntime::applyClip(const char *animationId) {
  if (!_loader || !animationId) {
    return;
  }
  const AnimationClip *clip = _assets.getAnimation(animationId);
  if (!clip) {
    clip = _assets.getAnimation("idle");
  }
  _clipPlayer.setClip(clip);
  _activeClipId = clip ? clip->id : "";
}

void CharacterRuntime::playAnimation(const char *animationId) {
  applyClip(animationId);
}

void CharacterRuntime::setState(const char *state) {
  _graph.setState(state);
  applyClip(_graph.currentAnimation());
}

void CharacterRuntime::setMessage(const char *text) {
  _message = text ? text : "";
}

void CharacterRuntime::setBackground(const char *backgroundKey) {
  if (!backgroundKey) {
    return;
  }
  if (strcmp(backgroundKey, "office") == 0 && _loader) {
    _backgroundSprite = _loader->defaultBackgroundSprite();
  } else {
    _backgroundSprite = backgroundKey;
  }
}

void CharacterRuntime::tick(unsigned long nowMs) {
  _clipPlayer.tick(nowMs);
  updateFps(nowMs);
}

void CharacterRuntime::updateFps(unsigned long nowMs) {
  _frameCount++;
  if (_lastFpsMs == 0) {
    _lastFpsMs = nowMs;
    return;
  }
  if (nowMs - _lastFpsMs >= 1000) {
    _fps = _frameCount;
    _frameCount = 0;
    _lastFpsMs = nowMs;
  }
}

void CharacterRuntime::render() {
  if (!_renderer || !_loader) {
    return;
  }
  const char *bodySprite = _clipPlayer.currentSpriteId();
  _compositor.render(*_renderer, *_loader, _cache, _assets, _backgroundSprite.c_str(), bodySprite,
                     _loader->anchorX(), _loader->anchorY(), _message.c_str(),
                     _activeClipId.c_str());
}
