#include "character_runtime.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

#include "ambient/display_mode.h"
#include "net/service_status.h"

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
  _brain.setActivity("idle");
  invalidateRender(DirtyFull);
}

void CharacterRuntime::invalidateRender(DirtyFlags flags) {
  _dirtyTracker.invalidate(flags);
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

  if (!loader.graphText().empty()) {
    if (!_graph.loadFromText(loader.graphText())) {
      _graph.applyDefaults();
    }
  } else if (!_graph.loadFromPack(loader.rootPath())) {
    _graph.applyDefaults();
  }

  _backgroundSprite = loader.defaultBackgroundSprite();
  _renderMode = RenderMode::Eyes;

  if (_renderer) {
    _renderer->setRotation(1);
  }
  _brain.loadFromPackPath(loader.rootPath().c_str());
  syncSpriteContext();
  syncClipFromBehavior();
  invalidateRender(DirtyFull);
  Serial.println("render_mode=eyes");
  return true;
}

void CharacterRuntime::unload() {
  _cache.clear();
  _characterId = "eyes";
  _activeClipId.clear();
  _bodySpriteId.clear();
  _loader = nullptr;
  _assets.bind(nullptr);
  _accessories.clear();
  _lastLoadError = CharacterLoadError::None;
  _overrideAnimation = false;
  _lastClipFrame = -1;
  if (_renderer) {
    _renderer->setRotation(0);
  }
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
  if (!clip) {
    return;
  }
  if (_activeClipId == clip->id) {
    return;
  }
  _clipPlayer.setClip(clip);
  _activeClipId = clip->id;
}

void CharacterRuntime::syncClipFromBehavior() {
  if (_overrideAnimation || !_loader) {
    return;
  }
  applyClip(_brain.clipForBehavior());
  const char *sprite = _clipPlayer.currentSpriteId();
  if (sprite) {
    _bodySpriteId = sprite;
  }
}

void CharacterRuntime::setActivity(const char *activity) {
  if (!activity || !activity[0]) {
    return;
  }
  _overrideAnimation = false;
  if (strcmp(activity, _brain.activity()) == 0) {
    _brain.forceBehaviorPick();
    syncClipFromBehavior();
    _dirtyTracker.forceDirty(DirtyBehavior | DirtyCharacter);
  } else {
    _brain.setActivity(activity);
    syncClipFromBehavior();
    _dirtyTracker.forceDirty(DirtyBehavior | DirtyCharacter);
  }
}

const char *CharacterRuntime::currentAnimation() const {
  if (_overrideAnimation && !_activeClipId.empty()) {
    return _activeClipId.c_str();
  }
  return _brain.behaviorId();
}

void CharacterRuntime::setWeatherDisplay(const char *icon, const char *tempLine,
                                         const char *conditionLine, const char *city,
                                         float tempC) {
  _weatherIcon = icon ? icon : "";
  _weatherText = tempLine ? tempLine : "";
  _weatherConditionText = conditionLine ? conditionLine : "";
  _weatherCityText = city ? city : "";
  _weatherTempC = tempC;
  if (tempC < 15.0f) {
    _eyeTint = 0x07FF;
  } else if (tempC < 25.0f) {
    _eyeTint = 0x07E0;
  } else if (tempC < 35.0f) {
    _eyeTint = 0xFFE0;
  } else {
    _eyeTint = 0xF800;
  }
  if (_weatherIcon == "rain" || _weatherIcon == "storm") {
    _bodySpriteId = "eyes_rain";
  }
  if (_ambientMode == AmbientDisplayMode::WeatherScreen) {
    _dirtyTracker.forceDirty(DirtyBehavior | DirtyCharacter | DirtyBackground);
  } else {
    _dirtyTracker.forceDirty(DirtyCharacter);
  }
}

void CharacterRuntime::setClock(const char *timeText, const char *dateText) {
  _clockText = timeText ? timeText : "";
  _clockDateText = dateText ? dateText : "";
  if (_ambientMode == AmbientDisplayMode::ClockScreen) {
    _dirtyTracker.forceDirty(DirtyBehavior | DirtyBackground);
  }
}

void CharacterRuntime::setDisplayMode(AmbientDisplayMode mode) {
  if (_ambientMode == mode) {
    return;
  }
  _ambientMode = mode;
  _transitionAlpha = 255;
  invalidateRender(DirtyFull);
}

void CharacterRuntime::setPomodoro(unsigned long remainingSec, unsigned long totalSec) {
  _pomodoroRemainingSec = remainingSec;
  _pomodoroTotalSec = totalSec;
  if (_ambientMode == AmbientDisplayMode::PomodoroScreen) {
    _dirtyTracker.forceDirty(DirtyFull);
  }
}

void CharacterRuntime::setStats(unsigned long uptimeSec, unsigned long heapFree, int wifiRssi,
                                const char *firmwareVersion) {
  _uptimeSec = uptimeSec;
  _heapFree = heapFree;
  _wifiRssi = wifiRssi;
  (void)firmwareVersion;
  if (_ambientMode == AmbientDisplayMode::StatsScreen) {
    _dirtyTracker.forceDirty(DirtyFull);
  }
}

void CharacterRuntime::setCalendarText(const char *text) {
  _calendarText = text ? text : "";
}

void CharacterRuntime::setMinigameText(const char *text) {
  _minigameText = text ? text : "";
  _dirtyTracker.forceDirty(DirtyBehavior);
}

void CharacterRuntime::triggerNotify(unsigned long durationMs) {
  _notifyUntilMs = millis() + durationMs;
  _dirtyTracker.forceDirty(DirtyCharacter);
}

void CharacterRuntime::setTransitionAlpha(uint8_t alpha) {
  if (_transitionAlpha == alpha) {
    return;
  }
  _transitionAlpha = alpha;
  if (alpha > 0) {
    _dirtyTracker.forceDirty(DirtyFull);
  }
}

void CharacterRuntime::setEyeTint(uint16_t rgb565) {
  _eyeTint = rgb565;
  _dirtyTracker.forceDirty(DirtyCharacter);
}

RenderState CharacterRuntime::buildRenderState() const {
  RenderState state;
  state.activity = _brain.activity();
  state.behaviorId = _brain.behaviorId();
  state.behaviorLabel = _brain.behaviorLabel();
  state.clockText = _clockText.empty() ? nullptr : _clockText.c_str();
  state.clockDateText = _clockDateText.empty() ? nullptr : _clockDateText.c_str();
  state.weatherText = _weatherText.empty() ? nullptr : _weatherText.c_str();
  state.weatherConditionText =
      _weatherConditionText.empty() ? nullptr : _weatherConditionText.c_str();
  state.weatherCityText = _weatherCityText.empty() ? nullptr : _weatherCityText.c_str();
  state.weatherIcon = _weatherIcon.empty() ? nullptr : _weatherIcon.c_str();
  state.weatherTempC = _weatherTempC;
  state.eyeTint = _notifyUntilMs > millis() ? 0xFD20 : _eyeTint;
  state.notifyFlash = _notifyUntilMs > millis();
  state.pomodoroRemainingSec = _pomodoroRemainingSec;
  state.pomodoroTotalSec = _pomodoroTotalSec;
  state.uptimeSec = _uptimeSec;
  state.heapFree = _heapFree;
  state.wifiRssi = _wifiRssi;
  state.firmwareVersion = NOMA_FIRMWARE_VERSION;
  state.calendarText = _calendarText.empty() ? nullptr : _calendarText.c_str();
  state.minigameText = _minigameText.empty() ? nullptr : _minigameText.c_str();
  state.transitionAlpha = _transitionAlpha;
  state.serviceStatus = collectServiceStatus();
  state.ambientMode = _ambientMode;
  state.backgroundSpriteId =
      _backgroundSprite.empty() ? nullptr : _backgroundSprite.c_str();
  state.bodySpriteId = _bodySpriteId.empty() ? nullptr : _bodySpriteId.c_str();
  state.clipFrameIndex = _clipPlayer.currentFrameIndex();
  return state;
}

DirtyFlags CharacterRuntime::collectDirtyFlags() {
  RenderState state = buildRenderState();
  DirtyFlags dirty = _dirtyTracker.collectDirtyFlags(state);
  if (_loader && _lastClipFrame >= 0 && state.clipFrameIndex != _lastClipFrame) {
    dirty = dirty | DirtyCharacter;
  }
  return dirty;
}

void CharacterRuntime::render(DirtyFlags dirty) {
  if (!anyDirty(dirty) || !_renderer || !_loader) {
    return;
  }

  unsigned long renderStart = millis();
  RenderState state = buildRenderState();
  _scheduler.render(state, dirty);
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
  if (!_overrideAnimation) {
    _brain.update(nowMs);
    syncClipFromBehavior();
  }
  if (_loader) {
    _clipPlayer.tick(nowMs);
    const char *sprite = _clipPlayer.currentSpriteId();
    if (sprite && _bodySpriteId != sprite) {
      _bodySpriteId = sprite;
      _dirtyTracker.forceDirty(DirtyCharacter);
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
