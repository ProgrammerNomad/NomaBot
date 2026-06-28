#pragma once

#include <stddef.h>
#include <stdint.h>

struct BehaviorDef {
  const char *id;
  const char *label;
  uint8_t weight;
  uint16_t durationMinSec;
  uint16_t durationMaxSec;
  const char *clip;
};

struct ActivityBehaviorTable {
  const char *activityId;
  const BehaviorDef *behaviors;
  size_t count;
};

struct EmotionBehaviorTable {
  const char *emotionId;
  const char *activityId;
  const BehaviorDef *behaviors;
  size_t count;
};

struct PersonalityTraits {
  int energy = 80;
  int coffeeLove = 90;
  int curiosity = 70;
  int sleepiness = 20;
  int optimism = 90;
  int patience = 40;
  int playfulness = 60;
};

namespace BehaviorDefaults {

extern const BehaviorDef kIdleBehaviors[];
extern const size_t kIdleBehaviorCount;

extern const BehaviorDef kCodingBehaviors[];
extern const size_t kCodingBehaviorCount;

extern const BehaviorDef kSleepBehaviors[];
extern const size_t kSleepBehaviorCount;

extern const ActivityBehaviorTable kActivityTables[];
extern const size_t kActivityTableCount;

extern const EmotionBehaviorTable kEmotionTables[];
extern const size_t kEmotionTableCount;

const BehaviorDef *findBehaviorClip(const char *behaviorId);

} // namespace BehaviorDefaults
