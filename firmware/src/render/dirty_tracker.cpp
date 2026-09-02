#include "dirty_tracker.h"

#include <string.h>

bool DirtyTracker::strChanged(const char *a, const char *b) {
  if (a == b) {
    return false;
  }
  if (!a) {
    a = "";
  }
  if (!b) {
    b = "";
  }
  return strcmp(a, b) != 0;
}

DirtyFlags DirtyTracker::computeDiff(const RenderState &next) const {
  DirtyFlags dirty = DirtyNone;

  if (strChanged(next.activity, _last.activity)) {
    dirty = dirty | DirtyHeader;
  }
  if (strChanged(next.behaviorId, _last.behaviorId) ||
      strChanged(next.behaviorLabel, _last.behaviorLabel)) {
    dirty = dirty | DirtyBehavior;
    if (strChanged(next.behaviorId, _last.behaviorId)) {
      dirty = dirty | DirtyCharacter;
    }
  }
  if (strChanged(next.backgroundSpriteId, _last.backgroundSpriteId)) {
    dirty = dirty | DirtyBackground;
  }
  if (strChanged(next.bodySpriteId, _last.bodySpriteId) ||
      next.clipFrameIndex != _last.clipFrameIndex) {
    dirty = dirty | DirtyCharacter;
  }
  if (next.ambientMode != _last.ambientMode) {
    dirty = dirty | DirtyBehavior | DirtyBackground | DirtyCharacter;
  }
  if (next.ambientMode == AmbientDisplayMode::ClockScreen &&
      (strChanged(next.clockText, _last.clockText) ||
       strChanged(next.clockDateText, _last.clockDateText))) {
    dirty = dirty | DirtyBehavior | DirtyBackground;
  }
  if (next.ambientMode == AmbientDisplayMode::WeatherScreen &&
      (strChanged(next.weatherText, _last.weatherText) ||
       strChanged(next.weatherConditionText, _last.weatherConditionText) ||
       strChanged(next.weatherCityText, _last.weatherCityText) ||
       strChanged(next.weatherIcon, _last.weatherIcon))) {
    dirty = dirty | DirtyBehavior | DirtyBackground | DirtyCharacter;
  }
  return dirty;
}

DirtyFlags DirtyTracker::collectDirtyFlags(const RenderState &next) {
  if (!_initialized || _forceFull) {
    _forceFull = false;
    _pending = DirtyNone;
    _forced = DirtyNone;
    return DirtyFull;
  }

  DirtyFlags dirty = computeDiff(next) | _pending | _forced;
  _pending = DirtyNone;
  _forced = DirtyNone;
  return dirty;
}

void DirtyTracker::invalidate(DirtyFlags flags) {
  if (flags == DirtyFull) {
    _initialized = false;
    _forceFull = true;
    _pending = DirtyNone;
    _forced = DirtyNone;
  }
}

void DirtyTracker::forceDirty(DirtyFlags flags) { _forced = _forced | flags; }

void DirtyTracker::notePending(DirtyFlags flags) { _pending = _pending | flags; }

void DirtyTracker::commitRendered(const RenderState &state) {
  _last = state;
  _initialized = true;
}
