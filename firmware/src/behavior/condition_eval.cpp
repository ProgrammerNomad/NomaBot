#include "condition_eval.h"

#include <string.h>

#include "behavior_defaults.h"

bool evaluateBehaviorRequires(const char *behaviorId, const RuntimeContext &ctx,
                              const PersonalityTraits &personality) {
  if (!behaviorId) {
    return true;
  }
  if (strcmp(behaviorId, "coffee") == 0) {
    if (personality.coffeeLove >= 50) {
      return true;
    }
    return ctx.energy < 40;
  }
  if (strcmp(behaviorId, "sleep") == 0) {
    return ctx.idleMinutes > 10;
  }
  return true;
}
