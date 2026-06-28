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
  _scheduler.begin(renderer);
  _brain.useDefaults();
  PersonalityTraits traits;
  _brain.setPersonality(traits);
  _brain.setLifeMode("work");
  _brain.setActivity("idle");
  invalidateRender(DirtyFull);
}

void CharacterRuntime::useBehaviorDefaults() {
  _brain.useDefaults();
  _brain.setLifeMode("work");
  _brain.setActivity("idle");
  _renderMode = RenderMode::Text;
  invalidateRender(DirtyFull);
}

void CharacterRuntime::invalidateRender(DirtyFlags flags) {
  _dirtyTracker.invalidate(flags);
}

bool CharacterRuntime::textModeActive() const {
  return _renderMode == RenderMode::Text || _loader == nullptr;
}

void CharacterRuntime::syncSpriteContext() {
  _scheduler.setSpriteContext(_loader, &_cache, &_assets, &_compositor);
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
  if (!behaviorFile && !behaviorPath.empty() && behaviorPath[0] == '/') {
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
  _brain.loadFromPackPath(loader.rootPath().c_str());
  syncSpriteContext();
  syncClipFromBehavior();
  invalidateRender(DirtyFull);
  return true;
}

void CharacterRuntime::unload() {
  _cache.clear();
  _characterId = "nomabot";
  _activeClipId.clear();
  _bodySpriteId.clear();
  _loader = nullptr;
  _assets.bind(nullptr);
  _accessories.clear();
  _lastLoadError = CharacterLoadError::None;
  _overrideAnimation = false;
  _lastClipFrame = -1;
  syncSpriteContext();
  invalidateRender(DirtyFull);
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
  applyClip(_brain.clipForBehavior());
  const char *sprite = _clipPlayer.currentSpriteId();
  if (sprite) {
    _bodySpriteId = sprite;
  }
}

void CharacterRuntime::noteCommandSource(const char *source) {
  if (source && source[0]) {
    _lastCommandSource = source;
  }
}

void CharacterRuntime::setLifeMode(const char *mode) {
  noteCommandSource("protocol");
  _brain.setLifeMode(mode);
  _dirtyTracker.forceDirty(DirtyHeader);
}

void CharacterRuntime::setActivity(const char *activity) {
  applyActivityCommand(activity, "protocol");
}

void CharacterRuntime::applyActivityCommand(const char *activity, const char *source) {
  if (!activity || !activity[0]) {
    return;
  }
  noteCommandSource(source ? source : "protocol");
  _overrideAnimation = false;
  if (strcmp(activity, _brain.activity()) == 0) {
    _brain.forceBehaviorPick();
    syncClipFromBehavior();
    _dirtyTracker.forceDirty(DirtyBehavior | DirtyMeta);
  } else {
    _brain.setActivity(activity);
    syncClipFromBehavior();
    _dirtyTracker.forceDirty(DirtyHeader | DirtyMeta | DirtyBehavior);
  }
}

void CharacterRuntime::setEmotion(const char *emotion) {
  noteCommandSource("protocol");
  _brain.setEmotion(emotion);
  syncClipFromBehavior();
  _dirtyTracker.forceDirty(DirtyMeta | DirtyBehavior);
}

void CharacterRuntime::setSeason(const char *season) {
  noteCommandSource("protocol");
  _brain.setSeason(season);
  _dirtyTracker.forceDirty(DirtyMeta);
}

void CharacterRuntime::triggerHabit(const char *habitId) {
  noteCommandSource("protocol");
  _brain.triggerHabit(habitId);
  syncClipFromBehavior();
  _dirtyTracker.forceDirty(DirtyBehavior | DirtyMeta | DirtyHeader);
}

void CharacterRuntime::playAnimation(const char *animationId) {
  noteCommandSource("protocol");
  _overrideAnimation = true;
  applyClip(animationId);
  _dirtyTracker.forceDirty(DirtyCharacter | DirtyBehavior);
}

void CharacterRuntime::setState(const char *state) { setActivity(state); }

const char *CharacterRuntime::currentAnimation() const {
  if (_overrideAnimation && !_activeClipId.empty()) {
    return _activeClipId.c_str();
  }
  return _brain.behaviorId();
}

void CharacterRuntime::setMessage(const char *id, const char *text, int priority,
                                  unsigned long durationMs) {
  noteCommandSource("protocol");
  _overlays.push(id, text, priority, durationMs, millis());
  _dirtyTracker.forceDirty(DirtyMessage);
}

void CharacterRuntime::setBackground(const char *backgroundKey) {
  if (!backgroundKey) {
    return;
  }
  noteCommandSource("protocol");
  if (strcmp(backgroundKey, "office") == 0 && _loader) {
    _backgroundSprite = _loader->defaultBackgroundSprite();
  } else {
    _backgroundSprite = backgroundKey;
  }
  _dirtyTracker.forceDirty(DirtyBackground);
}

RenderState CharacterRuntime::buildRenderState() const {
  RenderState state;
  state.lifeMode = _brain.lifeMode();
  state.activity = _brain.activity();
  state.emotion = _brain.emotion();
  state.goal = _brain.goal();
  state.goalProgress = _brain.goalProgress();
  state.behaviorId = _brain.behaviorId();
  state.behaviorLabel = _brain.behaviorLabel();
  state.energy = _brain.energy();
  state.displayEnergy = quantizeEnergy(state.energy);
  state.curiosity = _brain.curiosityActive();
  state.overlayText = _overlays.activeText();
  state.backgroundSpriteId =
      _backgroundSprite.empty() ? nullptr : _backgroundSprite.c_str();
  state.bodySpriteId = _bodySpriteId.empty() ? nullptr : _bodySpriteId.c_str();
  state.clipFrameIndex = _clipPlayer.currentFrameIndex();
  return state;
}

DirtyFlags CharacterRuntime::collectDirtyFlags() {
  RenderState state = buildRenderState();
  DirtyFlags dirty = _dirtyTracker.collectDirtyFlags(state);
  if (!textModeActive() && _lastClipFrame >= 0 &&
      state.clipFrameIndex != _lastClipFrame) {
    dirty = dirty | DirtyCharacter;
  }
  return dirty;
}

void CharacterRuntime::render(DirtyFlags dirty) {
  if (!anyDirty(dirty) || !_renderer) {
    return;
  }

  unsigned long renderStart = millis();
  RenderState state = buildRenderState();
  _scheduler.render(state, dirty, textModeActive());
  _dirtyTracker.commitRendered(state);
  _lastClipFrame = state.clipFrameIndex;
  _lastDirtyFlags = dirty;
  _renderCount++;
  _lastRenderMs = millis() - renderStart;
  updateFps(millis());
}

void CharacterRuntime::present() {
  DirtyFlags dirty = collectDirtyFlags();
  if (anyDirty(dirty)) {
    render(dirty);
  }
}

void CharacterRuntime::tick(unsigned long nowMs) {
  unsigned long tickStart = millis();
  if (_overlays.tick(nowMs)) {
    _dirtyTracker.notePending(DirtyMessage);
  }
  if (!_overrideAnimation) {
    _brain.update(nowMs);
    if (!textModeActive()) {
      syncClipFromBehavior();
    }
  }
  if (_loader && !textModeActive()) {
    _clipPlayer.tick(nowMs);
    const char *sprite = _clipPlayer.currentSpriteId();
    if (sprite) {
      _bodySpriteId = sprite;
    }
  }
  updateFps(nowMs);
  _lastBrainTickMs = millis() - tickStart;
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
