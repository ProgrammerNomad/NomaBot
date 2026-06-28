#pragma once

#include "renderer/renderer.hpp"

class TextSceneRenderer {
public:
  void render(IRenderer &renderer, const char *lifeMode, const char *activity,
              const char *emotion, const char *behaviorLabel, const char *messageText);
};
