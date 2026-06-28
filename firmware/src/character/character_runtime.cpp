#include "character_runtime.h"

#include <Arduino.h>

const char *characterLoadErrorLabel(CharacterLoadError err) {
  switch (err) {
  case CharacterLoadError::Graph:
    return "GRAPH FAIL";
  case CharacterLoadError::Pack:
    return "PACK FAIL";
  default:
    return "PACK FAIL";
  }
}

void CharacterRuntime::begin(IRenderer *renderer) {
  _renderer = renderer;
}

bool CharacterRuntime::loadCharacter(PackLoader &loader, const char *characterId) {
  unload();
  _lastLoadError = CharacterLoadError::None;

  if (!loader.load(characterId)) {
    _lastLoadError = CharacterLoadError::Pack;
    return false;
  }
  _loader = &loader;
  _assets.bind(&loader);
  _characterId = characterId;

  bool graphOk = false;
  if (!loader.graphText().empty()) {
    graphOk = _graph.loadFromText(loader.graphText());
  } else {
    graphOk = _graph.loadFromPack(loader.rootPath());
  }

  if (!graphOk) {
    Serial.println("Animation graph load failed — using idle/coding defaults");
    _graph.applyDefaults();
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
  _lastLoadError = CharacterLoadError::None;
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
