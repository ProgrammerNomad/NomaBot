#pragma once

#include <cstdint>

#include "ambient/display_mode.h"
#include "net/service_status.h"

struct RenderState {
  const char *activity = "idle";
  const char *behaviorId = "breathing";
  const char *behaviorLabel = "Breathing...";
  const char *clockText = "";
  const char *clockDateText = "";
  const char *weatherText = "";
  const char *weatherConditionText = "";
  const char *weatherCityText = "";
  const char *weatherIcon = "";
  AmbientDisplayMode ambientMode = AmbientDisplayMode::EyesAnim;
  const char *backgroundSpriteId = nullptr;
  const char *bodySpriteId = nullptr;
  int clipFrameIndex = 0;
  float weatherTempC = 20.0f;
  uint16_t eyeTint = 0x07FF;
  bool notifyFlash = false;
  unsigned long pomodoroRemainingSec = 0;
  unsigned long pomodoroTotalSec = 1500;
  unsigned long uptimeSec = 0;
  unsigned long heapFree = 0;
  int wifiRssi = 0;
  const char *firmwareVersion = nullptr;
  const char *calendarText = nullptr;
  const char *minigameText = nullptr;
  uint8_t transitionAlpha = 0;
  ServiceStatus serviceStatus{};
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
// DirtyBubble, DirtyHud - reuse bits only after ADR; not implemented in M5.

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

