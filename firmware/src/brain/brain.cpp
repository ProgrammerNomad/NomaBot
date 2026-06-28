#include "brain.h"
#include "brain/brain_loader.h"

#include <Arduino.h>
#include <string.h>

namespace {

const char *kMorningSeq[] = {"stretch", "coffee", "look_around"};
const char *kRecoverSeq[] = {"think", "sigh", "coffee", "typing_slow"};
const char *kDreamSeq[] = {"dream", "travel", "bike", "coffee"};
const char *kEveningSeq[] = {"look_around", "read", "smile"};

struct LabelEntry {
  const char *id;
  const char *label;
};

const LabelEntry kExtraLabels[] = {
    {"stretch", "Stretch"},       {"sigh", "Sigh..."},
    {"typing_slow", "Typing slowly..."}, {"celebrate", "Celebrate!"},
    {"read", "Reading..."},       {"travel", "Travel..."},
    {"bike", "Ride..."},          {"mountains", "Mountains..."},
};

} // namespace

void Brain::useDefaults() {
  _lifeMode = "work";
  _activity = "idle";
  _season = "spring";
  _goal = "none";
  _goalProgress = 0;
  _behaviorId = "breathing";
  _behaviorLabel = "Breathing...";
  _nextBehaviorId = "blink";
  _energy = 80;
  _boredom = 0;
  _pickMode = BrainPickMode::Weighted;
  _sequence = nullptr;
  _sequenceLen = 0;
  _sequenceIndex = 0;
  _emotion.setEmotion("neutral");
  _dayStartMs = millis();
}

bool Brain::loadFromPackPath(const char *rootPath) {
  if (!rootPath) {
    return false;
  }
  std::string path = std::string(rootPath) + "/behavior.json";
  return brainLoadFromJson(*this, path.c_str());
}

void Brain::setPersonality(const PersonalityTraits &traits) {
  _personality = traits;
  _energy = traits.energy;
  _runtime.energy = traits.energy;
}

void Brain::setLifeMode(const char *mode) {
  if (mode && mode[0]) {
    _lifeMode = mode;
    if (_lifeMode == "night") {
      _goal = "wind_down";
    }
  }
}

void Brain::setActivity(const char *activity) {
  if (!activity || !activity[0]) {
    return;
  }
  if (_activity != activity) {
    _activity = activity;
    _activityStartMs = millis();
    _runtime.idleMinutes = 0;
    if (_activity == "coding" && strcmp(_emotion.current(), "frustrated") != 0) {
      _goal = "focus";
      _goalProgress = 0;
    } else if (_activity == "idle") {
      _goal = "none";
      _goalProgress = 0;
    }
    pickBehavior(millis(), true);
  }
}

void Brain::forceBehaviorPick() { pickBehavior(millis(), true); }

void Brain::setEmotion(const char *emotion) {
  _emotion.setEmotion(emotion);
  if (emotion && strcmp(emotion, "frustrated") == 0) {
    _goal = "recover";
    _goalProgress = 0;
    enterSequence(kRecoverSeq, 4, "recover");
  } else {
    pickBehavior(millis(), true);
  }
}

void Brain::setSeason(const char *season) {
  if (season && season[0]) {
    _season = season;
  }
}

void Brain::triggerHabit(const char *habitId) {
  if (!habitId) {
    return;
  }
  if (strcmp(habitId, "morning") == 0) {
    enterSequence(kMorningSeq, 3, "morning_routine");
  } else if (strcmp(habitId, "tea_break") == 0) {
    setEmotion("happy");
    applyBehaviorId("coffee", millis());
  } else if (strcmp(habitId, "evening") == 0) {
    enterSequence(kEveningSeq, 3, "evening");
  } else if (strcmp(habitId, "dream") == 0) {
    enterSequence(kDreamSeq, 4, "dream");
  }
}

void Brain::noteBuildResult(const char *result) {
  if (result && strcmp(result, "failed") == 0) {
    setEmotion("frustrated");
  } else if (result && strcmp(result, "success") == 0) {
    setEmotion("excited");
    _goalProgress = 100;
    applyBehaviorId("celebrate", millis());
  }
}

const char *Brain::labelForId(const char *id) const {
  const BehaviorDef *def = BehaviorDefaults::findBehaviorClip(id);
  if (def && def->label) {
    return def->label;
  }
  for (const auto &e : kExtraLabels) {
    if (strcmp(e.id, id) == 0) {
      return e.label;
    }
  }
  return id;
}

void Brain::applyBehaviorId(const char *id, unsigned long nowMs) {
  if (!id) {
    return;
  }
  if (strcmp(id, _behaviorId.c_str()) == 0) {
    _boredom = min(100, _boredom + 12);
  } else {
    _boredom = max(0, _boredom - 8);
  }
  _behaviorId = id;
  _behaviorLabel = labelForId(id);
  const BehaviorDef *def = BehaviorDefaults::findBehaviorClip(id);
  if (def) {
    _behaviorDurationMs = randomDurationMs(*def);
  } else {
    _behaviorDurationMs = 5000;
  }
  _behaviorStartMs = nowMs;
  recordShortMemory(nowMs);
  pickNextPreview();
}

void Brain::enterSequence(const char **steps, size_t len, const char *goalName) {
  _pickMode = BrainPickMode::Sequence;
  _sequence = steps;
  _sequenceLen = len;
  _sequenceIndex = 0;
  _goal = goalName;
  if (len > 0) {
    applyBehaviorId(steps[0], millis());
  }
}

void Brain::advanceSequence(unsigned long nowMs) {
  if (_pickMode != BrainPickMode::Sequence || !_sequence || _sequenceLen == 0) {
    return;
  }
  if (nowMs - _behaviorStartMs < _behaviorDurationMs) {
    return;
  }
  _sequenceIndex++;
  if (_sequenceIndex >= _sequenceLen) {
    _pickMode = BrainPickMode::Weighted;
    _sequence = nullptr;
    _sequenceLen = 0;
    _sequenceIndex = 0;
    if (_goal == "recover") {
      _goal = "focus";
    } else {
      _goal = "none";
    }
    pickBehavior(nowMs, true);
    return;
  }
  applyBehaviorId(_sequence[_sequenceIndex], nowMs);
}

const BehaviorDef *Brain::activeTable(size_t *count) const {
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

int Brain::scaledWeight(const BehaviorDef &def) const {
  int weight = def.weight;
  if (strcmp(def.id, "coffee") == 0) {
    weight = weight * _personality.coffeeLove / 50;
  } else if (strcmp(def.id, "smile") == 0 || strcmp(def.id, "celebrate") == 0) {
    weight = weight * _personality.optimism / 50;
  } else if (strcmp(def.id, "look_around") == 0 || strcmp(def.id, "think") == 0 ||
             strcmp(def.id, "read") == 0) {
    weight = weight * _personality.curiosity / 50;
  } else if (strcmp(def.id, "yawn") == 0 || strcmp(def.id, "sleep") == 0) {
    weight = weight * _personality.sleepiness / 50;
  } else if (strcmp(def.id, "wave") == 0) {
    weight = weight * _personality.playfulness / 50;
  }
  if (_boredom > 40 &&
      (strcmp(def.id, "coffee") == 0 || strcmp(def.id, "stretch") == 0 ||
       strcmp(def.id, "look_around") == 0)) {
    weight = weight * 2;
  }
  if (_curiosityActive &&
      (strcmp(def.id, "look_around") == 0 || strcmp(def.id, "read") == 0)) {
    weight = weight * 3;
  }
  if (_season == "monsoon" &&
      (strcmp(def.id, "look_around") == 0 || strcmp(def.id, "read") == 0)) {
    weight = weight * 3 / 2;
  }
  if (weight < 1) {
    weight = 1;
  }
  return weight;
}

unsigned long Brain::randomDurationMs(const BehaviorDef &def) const {
  if (def.durationMaxSec <= def.durationMinSec) {
    return static_cast<unsigned long>(def.durationMinSec) * 1000UL;
  }
  uint16_t span = def.durationMaxSec - def.durationMinSec;
  uint16_t pick = def.durationMinSec + (esp_random() % (span + 1));
  return static_cast<unsigned long>(pick) * 1000UL;
}

void Brain::recordShortMemory(unsigned long nowMs) {
  if (_behaviorId == "coffee") {
    _lastCoffeeMs = nowMs;
  }
  if (_behaviorId == "sleep") {
    _lastSleepMs = nowMs;
  }
}

void Brain::pickBehavior(unsigned long nowMs, bool force) {
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
    applyBehaviorId(table[0].id, nowMs);
    return;
  }

  int roll = static_cast<int>(esp_random() % static_cast<uint32_t>(totalWeight));
  for (size_t i = 0; i < count; ++i) {
    if (!evaluateBehaviorRequires(table[i].id, _runtime, _personality)) {
      continue;
    }
    roll -= scaledWeight(table[i]);
    if (roll < 0) {
      if (!force && strcmp(table[i].id, _behaviorId.c_str()) == 0 && count > 1) {
        pickBehavior(nowMs, true);
        return;
      }
      applyBehaviorId(table[i].id, nowMs);
      return;
    }
  }
}

void Brain::pickNextPreview() {
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
    if (strcmp(table[i].id, _behaviorId.c_str()) != 0) {
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
    if (strcmp(table[i].id, _behaviorId.c_str()) == 0) {
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

void Brain::updateEnergy(unsigned long nowMs) {
  (void)nowMs;
  if (_activity == "coding") {
    _energy = max(5, _energy - 1);
  } else if (_behaviorId == "coffee" || _behaviorId == "stretch" || _behaviorId == "sleep") {
    _energy = min(100, _energy + 8);
  }
  _runtime.energy = _energy;
}

void Brain::updateCuriosity(unsigned long nowMs) {
  if (_lastCuriosityMs == 0) {
    _lastCuriosityMs = nowMs;
  }
  if (nowMs - _lastCuriosityMs > 45000UL && (esp_random() % 100) < _personality.curiosity / 2) {
    _curiosityActive = true;
    _lastCuriosityMs = nowMs;
    _behaviorLabel = "I wonder...";
  } else if (_curiosityActive && nowMs - _behaviorStartMs > 8000UL) {
    _curiosityActive = false;
  }
}

void Brain::updateGoal(unsigned long nowMs) {
  (void)nowMs;
  if (_goal != "focus" || _activity != "coding") {
    return;
  }
  if (_behaviorId == "typing" || _behaviorId == "think") {
    _goalProgress = min(100, _goalProgress + 8);
  }
  if (_goalProgress >= 100) {
    applyBehaviorId("celebrate", millis());
    _goalProgress = 0;
    _goal = "none";
  }
}

void Brain::maybeStartDream(unsigned long nowMs) {
  if (_lifeMode != "night" || _behaviorId != "sleep") {
    return;
  }
  if ((esp_random() % 100) < 5) {
    enterSequence(kDreamSeq, 4, "dream");
    (void)nowMs;
  }
}

void Brain::update(unsigned long nowMs) {
  _emotion.tick(nowMs);
  if (_activityStartMs > 0) {
    _runtime.idleMinutes = static_cast<int>((nowMs - _activityStartMs) / 60000UL);
  }
  updateEnergy(nowMs);
  updateCuriosity(nowMs);

  if (_pickMode == BrainPickMode::Sequence) {
    advanceSequence(nowMs);
    updateGoal(nowMs);
    return;
  }

  if (_behaviorStartMs == 0) {
    pickBehavior(nowMs, true);
    return;
  }
  if (nowMs - _behaviorStartMs >= _behaviorDurationMs) {
    maybeStartDream(nowMs);
    if (_pickMode == BrainPickMode::Sequence) {
      return;
    }
    pickBehavior(nowMs, true);
  }
  updateGoal(nowMs);
}

const char *Brain::clipForBehavior() const {
  const BehaviorDef *def = BehaviorDefaults::findBehaviorClip(_behaviorId.c_str());
  if (def && def->clip) {
    return def->clip;
  }
  return "idle";
}

int Brain::timeInBehaviorSec(unsigned long nowMs) const {
  if (_behaviorStartMs == 0) {
    return 0;
  }
  return static_cast<int>((nowMs - _behaviorStartMs) / 1000UL);
}

int Brain::lastCoffeeMinAgo(unsigned long nowMs) const {
  if (_lastCoffeeMs == 0) {
    return -1;
  }
  return static_cast<int>((nowMs - _lastCoffeeMs) / 60000UL);
}
