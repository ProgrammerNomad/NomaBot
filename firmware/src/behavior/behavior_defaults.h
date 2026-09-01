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

struct PersonalityTraits {
  int energy = 80;
  int curiosity = 70;
};

namespace BehaviorDefaults {

extern const BehaviorDef kIdleBehaviors[];
extern const size_t kIdleBehaviorCount;

extern const ActivityBehaviorTable kActivityTables[];
extern const size_t kActivityTableCount;

const BehaviorDef *findBehaviorClip(const char *behaviorId);

}  // namespace BehaviorDefaults
