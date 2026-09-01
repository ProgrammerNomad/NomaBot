#pragma once

enum class RenderMode {
  Text,
  Sprite,
  Eyes,
};

inline const char *renderModeName(RenderMode mode) {
  switch (mode) {
  case RenderMode::Sprite:
    return "sprite";
  case RenderMode::Eyes:
    return "eyes";
  default:
    return "text";
  }
}
