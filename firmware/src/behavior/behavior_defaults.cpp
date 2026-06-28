#include "behavior_defaults.h"

#include <string.h>

namespace BehaviorDefaults {

const BehaviorDef kIdleBehaviors[] = {
    {"look_around", "Look around", 30, 4, 12, "idle"},
    {"blink", "Blink", 25, 2, 5, "idle"},
    {"breathing", "Breathing...", 25, 5, 15, "idle"},
    {"wave", "Wave", 10, 2, 4, "idle"},
    {"smile", "Smile", 10, 2, 4, "idle"},
};

const BehaviorDef kCodingBehaviors[] = {
    {"typing", "Typing...", 70, 5, 15, "coding"},
    {"coffee", "Coffee", 10, 3, 8, "coffee"},
    {"think", "Thinking...", 10, 4, 10, "coding"},
    {"stretch", "Stretch", 5, 2, 5, "idle"},
    {"smile", "Smile", 5, 2, 4, "idle"},
};

const BehaviorDef kSleepBehaviors[] = {
    {"yawn", "Yawn...", 30, 3, 6, "sleep"},
    {"blink_slow", "Blink slowly...", 25, 4, 8, "sleep"},
    {"sleep", "Sleep", 30, 10, 30, "sleep"},
    {"dream", "Dream...", 15, 8, 20, "sleep"},
};

const BehaviorDef kFrustratedCoding[] = {
    {"think", "Thinking...", 30, 4, 10, "coding"},
    {"sigh", "Sigh...", 25, 3, 6, "idle"},
    {"coffee", "Coffee", 20, 3, 8, "coffee"},
    {"typing_slow", "Typing slowly...", 25, 5, 12, "coding"},
};

const BehaviorDef kHappyCoding[] = {
    {"typing", "Typing...", 50, 5, 15, "coding"},
    {"smile", "Smile", 25, 2, 4, "idle"},
    {"coffee", "Coffee", 25, 3, 8, "coffee"},
};

const BehaviorDef kHappyIdle[] = {
    {"wave", "Wave", 30, 2, 4, "idle"},
    {"look_around", "Look around", 30, 4, 12, "idle"},
    {"smile", "Smile", 40, 2, 4, "idle"},
};

const BehaviorDef kSleepyIdle[] = {
    {"yawn", "Yawn...", 40, 3, 6, "sleep"},
    {"blink_slow", "Blink slowly...", 30, 4, 8, "sleep"},
    {"sleep", "Sleep", 30, 10, 30, "sleep"},
};

const ActivityBehaviorTable kActivityTables[] = {
    {"idle", kIdleBehaviors, sizeof(kIdleBehaviors) / sizeof(kIdleBehaviors[0])},
    {"coding", kCodingBehaviors, sizeof(kCodingBehaviors) / sizeof(kCodingBehaviors[0])},
    {"sleep", kSleepBehaviors, sizeof(kSleepBehaviors) / sizeof(kSleepBehaviors[0])},
};

const EmotionBehaviorTable kEmotionTables[] = {
    {"frustrated", "coding", kFrustratedCoding,
     sizeof(kFrustratedCoding) / sizeof(kFrustratedCoding[0])},
    {"happy", "coding", kHappyCoding, sizeof(kHappyCoding) / sizeof(kHappyCoding[0])},
    {"happy", "idle", kHappyIdle, sizeof(kHappyIdle) / sizeof(kHappyIdle[0])},
    {"sleepy", "idle", kSleepyIdle, sizeof(kSleepyIdle) / sizeof(kSleepyIdle[0])},
};

const size_t kIdleBehaviorCount = sizeof(kIdleBehaviors) / sizeof(kIdleBehaviors[0]);
const size_t kCodingBehaviorCount = sizeof(kCodingBehaviors) / sizeof(kCodingBehaviors[0]);
const size_t kSleepBehaviorCount = sizeof(kSleepBehaviors) / sizeof(kSleepBehaviors[0]);
const size_t kActivityTableCount = sizeof(kActivityTables) / sizeof(kActivityTables[0]);
const size_t kEmotionTableCount = sizeof(kEmotionTables) / sizeof(kEmotionTables[0]);

const BehaviorDef *findBehaviorClip(const char *behaviorId) {
  if (!behaviorId) {
    return nullptr;
  }
  const ActivityBehaviorTable tables[] = {
      {"idle", kIdleBehaviors, kIdleBehaviorCount},
      {"coding", kCodingBehaviors, kCodingBehaviorCount},
      {"sleep", kSleepBehaviors, kSleepBehaviorCount},
  };
  for (const auto &table : tables) {
    for (size_t i = 0; i < table.count; ++i) {
      if (strcmp(table.behaviors[i].id, behaviorId) == 0) {
        return &table.behaviors[i];
      }
    }
  }
  for (size_t i = 0; i < kEmotionTableCount; ++i) {
    const auto &et = kEmotionTables[i];
    for (size_t j = 0; j < et.count; ++j) {
      if (strcmp(et.behaviors[j].id, behaviorId) == 0) {
        return &et.behaviors[j];
      }
    }
  }
  return nullptr;
}

} // namespace BehaviorDefaults
