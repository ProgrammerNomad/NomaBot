#pragma once

#include <string>

#include "behavior/behavior_defaults.h"
#include "behavior/pack_behavior_table.h"
#include "behavior/pack_clip_map.h"

class Brain {
public:
  void useDefaults();
  bool loadFromPackPath(const char *rootPath);
  void setPersonality(const PersonalityTraits &traits);

  void setActivity(const char *activity);
  void forceBehaviorPick();
  void update(unsigned long nowMs);

  const char *activity() const { return _activity.c_str(); }
  const char *behaviorId() const { return _behaviorId.c_str(); }
  const char *behaviorLabel() const { return _behaviorLabel.c_str(); }
  const char *nextBehaviorId() const { return _nextBehaviorId.c_str(); }
  const char *clipForBehavior() const;
  const char *clipForBehaviorId(const char *behaviorId) const;
  bool loadClipMapFromJsonText(const std::string &text);
  bool loadPackBehaviorsFromJsonText(const std::string &text);
  bool hasPackBehaviors() const { return _packBehaviorTable.hasActivities(); }
  int timeInBehaviorSec(unsigned long nowMs) const;

private:
  std::string _activity = "idle";
  std::string _behaviorId = "idle";
  std::string _behaviorLabel;
  std::string _nextBehaviorId = "blink";

  PersonalityTraits _personality;
  PackClipMap _clipMap;
  PackBehaviorTable _packBehaviorTable;

  unsigned long _behaviorStartMs = 0;
  unsigned long _behaviorDurationMs = 5000;
  unsigned long _activityStartMs = 0;

  void pickBehavior(unsigned long nowMs, bool force);
  void pickBehaviorFromPack(const std::vector<PackBehaviorEntry> &table, unsigned long nowMs,
                            bool force);
  void pickNextPreview();
  void pickNextPreviewFromPack(const std::vector<PackBehaviorEntry> &table);
  unsigned long randomDurationMs(const BehaviorDef &def) const;
  unsigned long randomDurationMs(const PackBehaviorEntry &entry) const;
  void applyBehaviorId(const char *id, unsigned long nowMs);
  const char *labelForId(const char *id) const;
  const BehaviorDef *activeTable(size_t *count) const;
};
