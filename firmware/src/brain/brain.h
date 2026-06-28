#pragma once

#include <string>

#include "behavior/behavior_defaults.h"
#include "behavior/condition_eval.h"
#include "behavior/emotion_state.h"

enum class BrainPickMode { Weighted, Sequence };

class Brain {
public:
  void useDefaults();
  bool loadFromPackPath(const char *rootPath);
  void setPersonality(const PersonalityTraits &traits);

  void setLifeMode(const char *mode);
  void setActivity(const char *activity);
  void forceBehaviorPick();
  void setEmotion(const char *emotion);
  void setSeason(const char *season);
  void triggerHabit(const char *habitId);
  void noteBuildResult(const char *result);

  void update(unsigned long nowMs);

  const char *lifeMode() const { return _lifeMode.c_str(); }
  const char *activity() const { return _activity.c_str(); }
  const char *emotion() const { return _emotion.current(); }
  const char *season() const { return _season.c_str(); }
  const char *goal() const { return _goal.c_str(); }
  int goalProgress() const { return _goalProgress; }
  int energy() const { return _energy; }
  int boredom() const { return _boredom; }
  bool curiosityActive() const { return _curiosityActive; }
  const char *behaviorId() const { return _behaviorId.c_str(); }
  const char *behaviorLabel() const { return _behaviorLabel.c_str(); }
  const char *nextBehaviorId() const { return _nextBehaviorId.c_str(); }
  const char *clipForBehavior() const;
  int timeInBehaviorSec(unsigned long nowMs) const;
  int lastCoffeeMinAgo(unsigned long nowMs) const;

private:
  std::string _lifeMode = "work";
  std::string _activity = "idle";
  std::string _season = "spring";
  std::string _goal = "none";
  std::string _behaviorId = "breathing";
  std::string _behaviorLabel = "Breathing...";
  std::string _nextBehaviorId = "blink";

  EmotionState _emotion;
  PersonalityTraits _personality;
  RuntimeContext _runtime;

  int _energy = 80;
  int _boredom = 0;
  int _goalProgress = 0;
  bool _curiosityActive = false;

  BrainPickMode _pickMode = BrainPickMode::Weighted;
  const char **_sequence = nullptr;
  size_t _sequenceLen = 0;
  size_t _sequenceIndex = 0;

  unsigned long _lastCoffeeMs = 0;
  unsigned long _lastSleepMs = 0;
  unsigned long _behaviorStartMs = 0;
  unsigned long _behaviorDurationMs = 5000;
  unsigned long _activityStartMs = 0;
  unsigned long _lastCuriosityMs = 0;
  unsigned long _dayStartMs = 0;

  void pickBehavior(unsigned long nowMs, bool force);
  void pickNextPreview();
  void enterSequence(const char **steps, size_t len, const char *goalName);
  void advanceSequence(unsigned long nowMs);
  void updateGoal(unsigned long nowMs);
  void updateEnergy(unsigned long nowMs);
  void updateCuriosity(unsigned long nowMs);
  void maybeStartDream(unsigned long nowMs);
  unsigned long randomDurationMs(const BehaviorDef &def) const;
  void applyBehaviorId(const char *id, unsigned long nowMs);
  const char *labelForId(const char *id) const;
  void recordShortMemory(unsigned long nowMs);
  const BehaviorDef *activeTable(size_t *count) const;
  int scaledWeight(const BehaviorDef &def) const;
};
