#include "asset_registry.h"

const SpriteMeta *AssetRegistry::getSprite(const char *id) const {
  return _loader ? _loader->findSprite(id) : nullptr;
}

const AnimationClip *AssetRegistry::getAnimation(const char *id) const {
  return _loader ? _loader->findAnimation(id) : nullptr;
}

BackgroundRef AssetRegistry::getBackground(const char *spriteId) const {
  BackgroundRef ref;
  ref.meta = getSprite(spriteId);
  return ref;
}

AccessoryRef AssetRegistry::getAccessory(const char *) const { return {}; }

FontRef AssetRegistry::getFont(const char *) const { return {}; }

SoundRef AssetRegistry::getSound(const char *) const { return {}; }
