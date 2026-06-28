#include "behavior_engine.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include <string.h>

#include "condition_eval.h"

struct BehaviorMemory {
  unsigned long lastCoffeeMs = 0;
  unsigned long lastSleepMs = 0;
  const char *lastBuild = nullptr;
};

static BehaviorMemory gMemory;

void BehaviorEngine::useDefaults() {
  _lifeMode = "work";
  _activity = "idle";
  _behaviorId = "breathing";
  _behaviorLabel = "Breathing...";
  _nextBehaviorId = "blink";
  _emotion.setEmotion("neutral");
}

void BehaviorEngine::setPersonality(const PersonalityTraits &traits) { _personality = traits; }

void BehaviorEngine::setLifeMode(const char *mode) {
  if (mode && mode[0]) {
    _lifeMode = mode;
  }
}

void BehaviorEngine::setActivity(const char *activity) {
  if (!activity || !activity[0]) {
    return;
  }
  if (_activity != activity) {
    _activity = activity;
    _activityStartMs = millis();
    _runtime.idleMinutes = 0;
    pickBehavior(millis(), true);
  }
}

void BehaviorEngine::setEmotion(const char *emotion) {
  _emotion.setEmotion(emotion);
  pickBehavior(millis(), true);
}

void BehaviorEngine::recordBehaviorMemory(unsigned long nowMs) {
  if (_behaviorId == "coffee") {
    gMemory.lastCoffeeMs = nowMs;
  }
  if (_behaviorId == "sleep") {
    gMemory.lastSleepMs = nowMs;
  }
}

const BehaviorDef *BehaviorEngine::activeTable(size_t *count) const {
  const char *emotion = _emotion.current();
  if (emotion && strcmp(emotion, "neutral") != 0) {
    for (size_t i = 0; i < BehaviorDefaults::kEmotionTableCount; ++i) {
      const auto &table = BehaviorDefaults::kEmotionTables[i];
      if (strcmp(table.emotionId, emotion) == 0 && strcmp(table.activityId, _activity.c_str()) == 0) {
        *count = table.count;
        return table.behaviors;
      }
    }
  }
  for (size_t i = 0; i < BehaviorDefaults::kActivityTableCount; ++i) {
    const auto &table = BehaviorDefaults::kActivityTables[i];
    if (strcmp(table.activityId, _activity.c_str()) == 0) {
      *count = table.count;
      return table.behaviors;
    }
  }
  *count = BehaviorDefaults::kIdleBehaviorCount;
  return BehaviorDefaults::kIdleBehaviors;
}

int BehaviorEngine::scaledWeight(const BehaviorDef &def) const {
  int weight = def.weight;
  if (strcmp(def.id, "coffee") == 0) {
    weight = weight * _personality.coffeeLove / 50;
  } else if (strcmp(def.id, "smile") == 0 || strcmp(def.id, "celebrate") == 0) {
    weight = weight * _personality.optimism / 50;
  } else if (strcmp(def.id, "look_around") == 0 || strcmp(def.id, "think") == 0) {
    weight = weight * _personality.curiosity / 50;
  } else if (strcmp(def.id, "yawn") == 0 || strcmp(def.id, "sleep") == 0) {
    weight = weight * _personality.sleepiness / 50;
  } else if (strcmp(def.id, "wave") == 0) {
    weight = weight * _personality.playfulness / 50;
  }
  if (weight < 1) {
    weight = 1;
  }
  return weight;
}

unsigned long BehaviorEngine::randomDurationMs(const BehaviorDef &def) const {
  if (def.durationMaxSec <= def.durationMinSec) {
    return static_cast<unsigned long>(def.durationMinSec) * 1000UL;
  }
  uint16_t span = def.durationMaxSec - def.durationMinSec;
  uint16_t pick = def.durationMinSec + (esp_random() % (span + 1));
  return static_cast<unsigned long>(pick) * 1000UL;
}

void BehaviorEngine::pickBehavior(unsigned long nowMs, bool force) {
  if (!force && _behaviorStartMs > 0 && nowMs - _behaviorStartMs < _behaviorDurationMs) {
    return;
  }

  size_t count = 0;
  const BehaviorDef *table = activeTable(&count);
  if (!table || count == 0) {
    return;
  }

  int totalWeight = 0;
  for (size_t i = 0; i < count; ++i) {
    if (!evaluateBehaviorRequires(table[i].id, _runtime, _personality)) {
      continue;
    }
    totalWeight += scaledWeight(table[i]);
  }
  if (totalWeight <= 0) {
    _behaviorId = table[0].id;
    _behaviorLabel = table[0].label;
    _behaviorDurationMs = randomDurationMs(table[0]);
    _behaviorStartMs = nowMs;
    recordBehaviorMemory(nowMs);
    pickNextPreview();
    return;
  }

  int roll = static_cast<int>(esp_random() % static_cast<uint32_t>(totalWeight));
  for (size_t i = 0; i < count; ++i) {
    if (!evaluateBehaviorRequires(table[i].id, _runtime, _personality)) {
      continue;
    }
    roll -= scaledWeight(table[i]);
    if (roll < 0) {
      if (!force && table[i].id == _behaviorId && count > 1) {
        pickBehavior(nowMs, true);
        return;
      }
      _behaviorId = table[i].id;
      _behaviorLabel = table[i].label;
      _behaviorDurationMs = randomDurationMs(table[i]);
      _behaviorStartMs = nowMs;
      recordBehaviorMemory(nowMs);
      pickNextPreview();
      return;
    }
  }
}

void BehaviorEngine::pickNextPreview() {
  size_t count = 0;
  const BehaviorDef *table = activeTable(&count);
  if (!table || count == 0) {
    _nextBehaviorId = _behaviorId;
    return;
  }
  int totalWeight = 0;
  for (size_t i = 0; i < count; ++i) {
    if (!evaluateBehaviorRequires(table[i].id, _runtime, _personality)) {
      continue;
    }
    if (table[i].id != _behaviorId) {
      totalWeight += scaledWeight(table[i]);
    }
  }
  if (totalWeight <= 0) {
    _nextBehaviorId = _behaviorId;
    return;
  }
  int roll = static_cast<int>(esp_random() % static_cast<uint32_t>(totalWeight));
  for (size_t i = 0; i < count; ++i) {
    if (!evaluateBehaviorRequires(table[i].id, _runtime, _personality)) {
      continue;
    }
    if (table[i].id == _behaviorId) {
      continue;
    }
    roll -= scaledWeight(table[i]);
    if (roll < 0) {
      _nextBehaviorId = table[i].id;
      return;
    }
  }
  _nextBehaviorId = table[0].id;
}

void BehaviorEngine::update(unsigned long nowMs) {
  _emotion.tick(nowMs);
  if (_activityStartMs > 0) {
    _runtime.idleMinutes = static_cast<int>((nowMs - _activityStartMs) / 60000UL);
  }
  if (_activity == "idle") {
    _runtime.energy = max(10, _personality.energy - _runtime.idleMinutes);
  } else {
    _runtime.energy = _personality.energy;
  }

  if (_behaviorStartMs == 0) {
    pickBehavior(nowMs, true);
    return;
  }
  if (nowMs - _behaviorStartMs >= _behaviorDurationMs) {
    pickBehavior(nowMs, true);
  }
  _lastUpdateMs = nowMs;
}

const char *BehaviorEngine::clipForBehavior() const {
  const BehaviorDef *def = BehaviorDefaults::findBehaviorClip(_behaviorId.c_str());
  if (def && def->clip) {
    return def->clip;
  }
  return "idle";
}

int BehaviorEngine::timeInBehaviorSec(unsigned long nowMs) const {
  if (_behaviorStartMs == 0) {
    return 0;
  }
  return static_cast<int>((nowMs - _behaviorStartMs) / 1000UL);
}
