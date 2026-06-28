#pragma once

#include <stddef.h>
#include <stdint.h>

#include "behavior_defaults.h"

struct RuntimeContext {
  int energy = 80;
  int coffeeLove = 90;
  int idleMinutes = 0;
};

bool evaluateBehaviorRequires(const char *behaviorId, const RuntimeContext &ctx,
                              const PersonalityTraits &personality);
