#pragma once

#include "behavior_defaults.h"

struct RuntimeContext {
  int energy = 80;
  int idleMinutes = 0;
};

bool evaluateBehaviorRequires(const char *behaviorId, const RuntimeContext &ctx,
                              const PersonalityTraits &personality);
