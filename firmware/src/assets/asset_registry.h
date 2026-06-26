#pragma once

#include "assets/pack_loader.h"

struct BackgroundRef {
  const SpriteMeta *meta = nullptr;
};

struct AccessoryRef {
  const char *id = nullptr;
};

struct FontRef {
  const char *id = nullptr;
};

struct SoundRef {
  const char *id = nullptr;
};

class AssetRegistry {
public:
  void bind(PackLoader *loader) { _loader = loader; }

  const SpriteMeta *getSprite(const char *id) const;
  const AnimationClip *getAnimation(const char *id) const;
  BackgroundRef getBackground(const char *spriteId) const;
  AccessoryRef getAccessory(const char *id) const;
  FontRef getFont(const char *id) const;
  SoundRef getSound(const char *id) const;

private:
  PackLoader *_loader = nullptr;
};
