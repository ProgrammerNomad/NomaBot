#pragma once

#include "behavior/behavior_defaults.h"

class Brain;

bool brainLoadFromJson(Brain &brain, const char *jsonPath);
