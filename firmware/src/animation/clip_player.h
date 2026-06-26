#pragma once

#include "assets/pack_loader.h"

class ClipPlayer {
public:
  void setClip(const AnimationClip *clip);
  void tick(unsigned long nowMs);
  void reset();

  const char *clipId() const { return _clip ? _clip->id.c_str() : ""; }
  const char *currentSpriteId() const;
  int currentFrameIndex() const { return _frameIndex; }

private:
  const AnimationClip *_clip = nullptr;
  int _frameIndex = 0;
  unsigned long _frameStartMs = 0;
  unsigned long _lastTickMs = 0;
};
