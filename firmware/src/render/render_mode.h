#pragma once

enum class RenderMode {
  Text,
  Sprite,
};

inline const char *renderModeName(RenderMode mode) {
  switch (mode) {
  case RenderMode::Sprite:
    return "sprite";
  default:
    return "text";
  }
}
