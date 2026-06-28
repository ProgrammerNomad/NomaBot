#pragma once

#include <cstdint>

struct RenderState {
  const char *lifeMode = "work";
  const char *activity = "idle";
  const char *emotion = "neutral";
  const char *goal = "none";
  int goalProgress = 0;
  const char *behaviorId = "breathing";
  const char *behaviorLabel = "Breathing...";
  int energy = 80;
  int displayEnergy = 80;
  bool curiosity = false;
  const char *overlayText = "";
  const char *backgroundSpriteId = nullptr;
  const char *bodySpriteId = nullptr;
  int clipFrameIndex = 0;
};

enum RenderLayer : uint8_t {
  RenderLayerBackground = 0,
  RenderLayerCharacter,
  RenderLayerOverlay,
  RenderLayerHud,
  RenderLayerDebug,
};

enum DirtyFlags : uint8_t {
  DirtyNone = 0,
  DirtyHeader = 1 << 0,
  DirtyMeta = 1 << 1,
  DirtyEnergy = 1 << 2,
  DirtyBehavior = 1 << 3,
  DirtyMessage = 1 << 4,
  DirtyCharacter = 1 << 5,
  DirtyBackground = 1 << 6,
  DirtyFull = 0xFF,
};

// M5 active: DirtyCharacter covers the whole character entity.
// Reserved M6+ (see docs/SCENE_SPEC.md): DirtyBody, DirtyEyes, DirtyAccessory,
// DirtyBubble, DirtyHud — reuse bits only after ADR; not implemented in M5.

inline DirtyFlags operator|(DirtyFlags a, DirtyFlags b) {
  return static_cast<DirtyFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline DirtyFlags operator&(DirtyFlags a, DirtyFlags b) {
  return static_cast<DirtyFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline bool anyDirty(DirtyFlags flags) { return flags != DirtyNone; }

inline bool hasDirty(DirtyFlags flags, DirtyFlags bit) {
  return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(bit)) != 0;
}

inline int quantizeEnergy(int energy) {
  if (energy <= 0) {
    return 0;
  }
  if (energy >= 100) {
    return 100;
  }
  return ((energy + 2) / 5) * 5;
}
