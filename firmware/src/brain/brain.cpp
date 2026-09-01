#include "brain.h"
#include "brain/brain_loader.h"

#include <Arduino.h>
#include <string.h>

void Brain::useDefaults() {
  _activity = "idle";
  _behaviorId = "idle";
  _behaviorLabel = "";
  _nextBehaviorId = "blink";
  _packBehaviorTable.clear();
}

bool Brain::loadFromPackPath(const char *rootPath) {
  if (!rootPath) {
    return false;
  }
  std::string path = std::string(rootPath) + "/behavior.json";
  return brainLoadFromJson(*this, path.c_str());
}

void Brain::setPersonality(const PersonalityTraits &traits) { _personality = traits; }

void Brain::setActivity(const char *activity) {
  if (!activity || !activity[0]) {
    return;
  }
  if (_activity != activity) {
    _activity = activity;
    _activityStartMs = millis();
    pickBehavior(millis(), true);
  }
}

void Brain::forceBehaviorPick() { pickBehavior(millis(), true); }

const char *Brain::labelForId(const char *id) const {
  if (const PackBehaviorEntry *packEntry = _packBehaviorTable.findBehavior(id)) {
    if (!packEntry->label.empty()) {
      return packEntry->label.c_str();
    }
    return id;
  }
  const BehaviorDef *def = BehaviorDefaults::findBehaviorClip(id);
  if (def && def->label && def->label[0]) {
    return def->label;
  }
  return id;
}

void Brain::applyBehaviorId(const char *id, unsigned long nowMs) {
  if (!id) {
    return;
  }
  _behaviorId = id;
  const PackBehaviorEntry *packEntry = _packBehaviorTable.findBehavior(id);
  if (packEntry) {
    _behaviorLabel = packEntry->label.empty() ? id : packEntry->label;
    _behaviorDurationMs = randomDurationMs(*packEntry);
  } else {
    _behaviorLabel = labelForId(id);
    const BehaviorDef *def = BehaviorDefaults::findBehaviorClip(id);
    if (def) {
      _behaviorDurationMs = randomDurationMs(*def);
    } else {
      _behaviorDurationMs = 5000;
    }
  }
  _behaviorStartMs = nowMs;
  pickNextPreview();
}

const BehaviorDef *Brain::activeTable(size_t *count) const {
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

unsigned long Brain::randomDurationMs(const BehaviorDef &def) const {
  if (def.durationMaxSec <= def.durationMinSec) {
    return static_cast<unsigned long>(def.durationMinSec) * 1000UL;
  }
  uint16_t span = def.durationMaxSec - def.durationMinSec;
  uint16_t pick = def.durationMinSec + (esp_random() % (span + 1));
  return static_cast<unsigned long>(pick) * 1000UL;
}

unsigned long Brain::randomDurationMs(const PackBehaviorEntry &entry) const {
  if (entry.durationMaxSec <= entry.durationMinSec) {
    return static_cast<unsigned long>(entry.durationMinSec) * 1000UL;
  }
  uint16_t span = entry.durationMaxSec - entry.durationMinSec;
  uint16_t pick = entry.durationMinSec + (esp_random() % (span + 1));
  return static_cast<unsigned long>(pick) * 1000UL;
}

void Brain::pickBehaviorFromPack(const std::vector<PackBehaviorEntry> &table, unsigned long nowMs,
                                 bool force) {
  if (table.empty()) {
    return;
  }

  int totalWeight = 0;
  for (const auto &entry : table) {
    if (force && entry.id == _behaviorId && table.size() > 1) {
      continue;
    }
    totalWeight += entry.weight;
  }
  if (totalWeight <= 0) {
    applyBehaviorId(table[0].id.c_str(), nowMs);
    return;
  }

  int roll = static_cast<int>(esp_random() % static_cast<uint32_t>(totalWeight));
  for (const auto &entry : table) {
    if (force && entry.id == _behaviorId && table.size() > 1) {
      continue;
    }
    roll -= entry.weight;
    if (roll < 0) {
      applyBehaviorId(entry.id.c_str(), nowMs);
      return;
    }
  }
}

void Brain::pickBehavior(unsigned long nowMs, bool force) {
  if (!force && _behaviorStartMs > 0 && nowMs - _behaviorStartMs < _behaviorDurationMs) {
    return;
  }

  if (const std::vector<PackBehaviorEntry> *packTable =
          _packBehaviorTable.behaviorsForActivity(_activity.c_str())) {
    pickBehaviorFromPack(*packTable, nowMs, force);
    return;
  }

  size_t count = 0;
  const BehaviorDef *table = activeTable(&count);
  if (!table || count == 0) {
    return;
  }

  int totalWeight = 0;
  for (size_t i = 0; i < count; ++i) {
    totalWeight += table[i].weight;
  }
  if (totalWeight <= 0) {
    applyBehaviorId(table[0].id, nowMs);
    return;
  }

  int roll = static_cast<int>(esp_random() % static_cast<uint32_t>(totalWeight));
  for (size_t i = 0; i < count; ++i) {
    roll -= table[i].weight;
    if (roll < 0) {
      applyBehaviorId(table[i].id, nowMs);
      return;
    }
  }
}

void Brain::pickNextPreviewFromPack(const std::vector<PackBehaviorEntry> &table) {
  int totalWeight = 0;
  for (const auto &entry : table) {
    if (entry.id != _behaviorId) {
      totalWeight += entry.weight;
    }
  }
  if (totalWeight <= 0) {
    _nextBehaviorId = _behaviorId;
    return;
  }
  int roll = static_cast<int>(esp_random() % static_cast<uint32_t>(totalWeight));
  for (const auto &entry : table) {
    if (entry.id == _behaviorId) {
      continue;
    }
    roll -= entry.weight;
    if (roll < 0) {
      _nextBehaviorId = entry.id;
      return;
    }
  }
  _nextBehaviorId = table[0].id;
}

void Brain::pickNextPreview() {
  if (const std::vector<PackBehaviorEntry> *packTable =
          _packBehaviorTable.behaviorsForActivity(_activity.c_str())) {
    pickNextPreviewFromPack(*packTable);
    return;
  }

  size_t count = 0;
  const BehaviorDef *table = activeTable(&count);
  if (!table || count == 0) {
    _nextBehaviorId = _behaviorId;
    return;
  }
  int totalWeight = 0;
  for (size_t i = 0; i < count; ++i) {
    if (strcmp(table[i].id, _behaviorId.c_str()) != 0) {
      totalWeight += table[i].weight;
    }
  }
  if (totalWeight <= 0) {
    _nextBehaviorId = _behaviorId;
    return;
  }
  int roll = static_cast<int>(esp_random() % static_cast<uint32_t>(totalWeight));
  for (size_t i = 0; i < count; ++i) {
    if (strcmp(table[i].id, _behaviorId.c_str()) == 0) {
      continue;
    }
    roll -= table[i].weight;
    if (roll < 0) {
      _nextBehaviorId = table[i].id;
      return;
    }
  }
  _nextBehaviorId = table[0].id;
}

void Brain::update(unsigned long nowMs) {
  if (_behaviorStartMs == 0) {
    pickBehavior(nowMs, true);
    return;
  }
  if (nowMs - _behaviorStartMs >= _behaviorDurationMs) {
    pickBehavior(nowMs, true);
  }
}

bool Brain::loadClipMapFromJsonText(const std::string &text) {
  return _clipMap.loadFromJsonText(text);
}

bool Brain::loadPackBehaviorsFromJsonText(const std::string &text) {
  return _packBehaviorTable.loadFromJsonText(text);
}

const char *Brain::clipForBehaviorId(const char *behaviorId) const {
  if (const PackBehaviorEntry *packEntry = _packBehaviorTable.findBehavior(behaviorId)) {
    if (!packEntry->clip.empty()) {
      return packEntry->clip.c_str();
    }
  }
  if (const char *packClip = _clipMap.clipForBehavior(behaviorId)) {
    return packClip;
  }
  const BehaviorDef *def = BehaviorDefaults::findBehaviorClip(behaviorId);
  if (def && def->clip) {
    return def->clip;
  }
  return "idle";
}

const char *Brain::clipForBehavior() const {
  return clipForBehaviorId(_behaviorId.c_str());
}

int Brain::timeInBehaviorSec(unsigned long nowMs) const {
  if (_behaviorStartMs == 0) {
    return 0;
  }
  return static_cast<int>((nowMs - _behaviorStartMs) / 1000UL);
}
