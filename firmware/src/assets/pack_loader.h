#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class PackLoadError {
  None,
  Manifest,
  Config,
  Animations,
};

const char *packLoadErrorLabel(PackLoadError err);

struct SpriteMeta {
  std::string id;
  std::string file;
  int width = 0;
  int height = 0;
  int bytes = 0;
};

struct ClipFrame {
  std::string spriteId;
  int durationMs = 500;
};

struct AnimationClip {
  std::string id;
  bool loop = true;
  std::vector<ClipFrame> frames;
};

struct PackVersion {
  int major = 0;
  int minor = 0;
  int patch = 0;
};

struct PackInfo {
  std::string packId;
  std::string uuid;
  PackVersion version;
  std::string profile;
  int displayWidth = 170;
  int displayHeight = 320;
};

class PackLoader {
public:
  bool mountFilesystem();
  bool load(const char *characterId);
  void unload();

  bool isLoaded() const { return _loaded; }
  PackLoadError lastError() const { return _lastError; }
  const char *characterId() const { return _characterId.c_str(); }
  const PackInfo &info() const { return _info; }
  const std::string &rootPath() const { return _root; }
  const std::string &graphText() const { return _graphText; }

  const SpriteMeta *findSprite(const char *id) const;
  bool readSpritePixels(const SpriteMeta *meta, uint16_t *buffer, size_t bufferBytes) const;
  const AnimationClip *findAnimation(const char *id) const;

  const char *defaultBackgroundSprite() const { return _defaultBgSprite.c_str(); }
  const char *expressionForEmotion(const char *emotion) const;
  int anchorX() const { return _anchorX; }
  int anchorY() const { return _anchorY; }

  void listDirectory(const std::string &path) const;

private:
  bool _loaded = false;
  PackLoadError _lastError = PackLoadError::None;
  std::string _characterId;
  std::string _root;
  std::string _graphText;
  PackInfo _info;
  std::string _defaultBgSprite;
  int _anchorX = 85;
  int _anchorY = 80;
  std::string _defaultExpressionSprite;
  std::vector<std::pair<std::string, std::string>> _expressions;
  std::vector<SpriteMeta> _sprites;
  std::vector<AnimationClip> _animations;

  bool loadManifest();
  bool loadConfig();
  bool loadAnimations();
  bool loadAnimationById(const std::string &animId);
  bool parseAnimationJson(const std::string &text, const char *fallbackId);
  std::string readTextFile(const std::string &path) const;
};
