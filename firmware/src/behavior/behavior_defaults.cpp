#include "behavior_defaults.h"

#include <string.h>

namespace BehaviorDefaults {

const BehaviorDef kIdleBehaviors[] = {
    {"idle", "", 50, 3, 8, "idle"},
    {"blink", "", 30, 1, 3, "blink"},
};

const size_t kIdleBehaviorCount = sizeof(kIdleBehaviors) / sizeof(kIdleBehaviors[0]);

const ActivityBehaviorTable kActivityTables[] = {
    {"idle", kIdleBehaviors, kIdleBehaviorCount},
};

const size_t kActivityTableCount = sizeof(kActivityTables) / sizeof(kActivityTables[0]);

const BehaviorDef *findBehaviorClip(const char *behaviorId) {
  if (!behaviorId) {
    return nullptr;
  }
  for (size_t i = 0; i < kIdleBehaviorCount; ++i) {
    if (strcmp(kIdleBehaviors[i].id, behaviorId) == 0) {
      return &kIdleBehaviors[i];
    }
  }
  return nullptr;
}

}  // namespace BehaviorDefaults
