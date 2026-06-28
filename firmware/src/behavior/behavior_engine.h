#pragma once

#include <string>

#include "behavior_defaults.h"
#include "condition_eval.h"
#include "emotion_state.h"

class BehaviorEngine {
public:
  void useDefaults();
  void setPersonality(const PersonalityTraits &traits);

  void setLifeMode(const char *mode);
  void setActivity(const char *activity);
  void setEmotion(const char *emotion);

  void update(unsigned long nowMs);

  const char *lifeMode() const { return _lifeMode.c_str(); }
  const char *activity() const { return _activity.c_str(); }
  const char *emotion() const { return _emotion.current(); }
  const char *behaviorId() const { return _behaviorId.c_str(); }
  const char *behaviorLabel() const { return _behaviorLabel.c_str(); }
  const char *nextBehaviorId() const { return _nextBehaviorId.c_str(); }
  const char *clipForBehavior() const;
  int timeInBehaviorSec(unsigned long nowMs) const;

private:
  std::string _lifeMode = "work";
  std::string _activity = "idle";
  std::string _behaviorId = "breathing";
  std::string _behaviorLabel = "Breathing...";
  std::string _nextBehaviorId = "blink";

  EmotionState _emotion;
  PersonalityTraits _personality;
  RuntimeContext _runtime;

  unsigned long _behaviorStartMs = 0;
  unsigned long _behaviorDurationMs = 5000;
  unsigned long _activityStartMs = 0;
  unsigned long _lastUpdateMs = 0;

  const BehaviorDef *activeTable(size_t *count) const;
  int scaledWeight(const BehaviorDef &def) const;
  void pickBehavior(unsigned long nowMs, bool force);
  void pickNextPreview();
  unsigned long randomDurationMs(const BehaviorDef &def) const;
  void recordBehaviorMemory(unsigned long nowMs);
};
