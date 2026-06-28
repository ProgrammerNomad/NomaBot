#include "character_runtime.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

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
  _behavior.useDefaults();
  PersonalityTraits traits;
  _behavior.setPersonality(traits);
  _behavior.setLifeMode("work");
  _behavior.setActivity("idle");
}

void CharacterRuntime::useBehaviorDefaults() {
  _behavior.useDefaults();
  _behavior.setLifeMode("work");
  _behavior.setActivity("idle");
  _renderMode = RenderMode::Text;
}

bool CharacterRuntime::textModeActive() const {
  return _renderMode == RenderMode::Text || _loader == nullptr;
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
  _renderMode = RenderMode::Text;

  std::string behaviorPath = loader.rootPath() + "/behavior.json";
  File behaviorFile = LittleFS.open(behaviorPath.c_str(), "r");
  if (!behaviorFile && behaviorPath.size() > 0 && behaviorPath[0] == '/') {
    behaviorFile = LittleFS.open(behaviorPath.c_str() + 1, "r");
  }
  if (behaviorFile) {
    std::string behaviorText;
    while (behaviorFile.available()) {
      behaviorText += static_cast<char>(behaviorFile.read());
    }
    behaviorFile.close();
    StaticJsonDocument<256> behaviorDoc;
    if (!deserializeJson(behaviorDoc, behaviorText)) {
      const char *mode = behaviorDoc["render_mode"] | "text";
      if (strcmp(mode, "sprite") == 0) {
        _renderMode = RenderMode::Sprite;
      }
    }
  }

  syncClipFromBehavior();
  return true;
}

void CharacterRuntime::unload() {
  _cache.clear();
  _characterId = "nomabot";
  _message.clear();
  _activeClipId.clear();
  _loader = nullptr;
  _assets.bind(nullptr);
  _accessories.clear();
  _lastLoadError = CharacterLoadError::None;
  _overrideAnimation = false;
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

void CharacterRuntime::syncClipFromBehavior() {
  if (_overrideAnimation || textModeActive()) {
    return;
  }
  applyClip(_behavior.clipForBehavior());
}

void CharacterRuntime::setLifeMode(const char *mode) {
  _behavior.setLifeMode(mode);
}

void CharacterRuntime::setActivity(const char *activity) {
  _overrideAnimation = false;
  _behavior.setActivity(activity);
  syncClipFromBehavior();
}

void CharacterRuntime::setEmotion(const char *emotion) {
  _behavior.setEmotion(emotion);
  syncClipFromBehavior();
}

void CharacterRuntime::playAnimation(const char *animationId) {
  _overrideAnimation = true;
  applyClip(animationId);
}

void CharacterRuntime::setState(const char *state) {
  setActivity(state);
}

const char *CharacterRuntime::currentAnimation() const {
  if (_overrideAnimation && !_activeClipId.empty()) {
    return _activeClipId.c_str();
  }
  return _behavior.behaviorId();
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
  if (!_overrideAnimation) {
    _behavior.update(nowMs);
    if (!textModeActive()) {
      syncClipFromBehavior();
    }
  }
  if (_loader && !textModeActive()) {
    _clipPlayer.tick(nowMs);
  }
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
  if (!_renderer) {
    return;
  }

  if (textModeActive()) {
    _textRenderer.render(*_renderer, _behavior.lifeMode(), _behavior.activity(),
                         _behavior.emotion(), _behavior.behaviorLabel(), _message.c_str());
    return;
  }

  if (!_loader) {
    _textRenderer.render(*_renderer, _behavior.lifeMode(), _behavior.activity(),
                         _behavior.emotion(), _behavior.behaviorLabel(), _message.c_str());
    return;
  }

  const char *bodySprite = _clipPlayer.currentSpriteId();
  _compositor.render(*_renderer, *_loader, _cache, _assets, _backgroundSprite.c_str(), bodySprite,
                     _loader->anchorX(), _loader->anchorY(), _message.c_str(),
                     _behavior.behaviorLabel());
}
