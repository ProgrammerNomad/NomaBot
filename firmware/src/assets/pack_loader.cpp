#include "pack_loader.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

const char *packLoadErrorLabel(PackLoadError err) {
  switch (err) {
  case PackLoadError::Manifest:
    return "MANIFEST FAIL";
  case PackLoadError::Config:
    return "CONFIG FAIL";
  case PackLoadError::Animations:
    return "ANIM FAIL";
  default:
    return "PACK FAIL";
  }
}

bool PackLoader::mountFilesystem() {
  // Partition name in partitions.csv is "littlefs" (not Arduino default "spiffs")
  if (!LittleFS.begin(true, "/littlefs", 10, "littlefs")) {
    Serial.println("LittleFS mount failed (label=littlefs)");
    return false;
  }
  Serial.println("LittleFS mounted OK");
  return true;
}

std::string PackLoader::readTextFile(const std::string &path) const {
  auto openPath = [](const std::string &p) -> File {
    File f = LittleFS.open(p.c_str(), "r");
    if (f) {
      return f;
    }
    if (!p.empty() && p[0] == '/') {
      return LittleFS.open(p.c_str() + 1, "r");
    }
    return File();
  };

  File f = openPath(path);
  if (!f) {
    return {};
  }
  std::string content;
  content.reserve(f.size() > 0 ? static_cast<size_t>(f.size()) : 512);
  while (f.available()) {
    content += static_cast<char>(f.read());
  }
  f.close();
  return content;
}

void PackLoader::listDirectory(const std::string &path) const {
  Serial.printf("FS listing %s:\n", path.c_str());
  File dir = LittleFS.open(path.c_str(), "r");
  if (!dir && !path.empty() && path[0] == '/') {
    dir = LittleFS.open(path.c_str() + 1, "r");
  }
  if (!dir) {
    Serial.println("  (cannot open)");
    return;
  }
  if (!dir.isDirectory()) {
    Serial.println("  (not a directory)");
    dir.close();
    return;
  }
  File entry = dir.openNextFile();
  while (entry) {
    Serial.printf("  %s %s\n", entry.isDirectory() ? "[dir]" : "[file]", entry.name());
    entry.close();
    entry = dir.openNextFile();
  }
  dir.close();
}

void PackLoader::unload() {
  _loaded = false;
  _lastError = PackLoadError::None;
  _characterId.clear();
  _root.clear();
  _graphText.clear();
  _sprites.clear();
  _animations.clear();
  _defaultBgSprite.clear();
}

bool PackLoader::load(const char *characterId) {
  unload();
  _characterId = characterId;
  _root = std::string("/characters/") + characterId;

  if (!loadManifest()) {
    _lastError = PackLoadError::Manifest;
    Serial.printf("Pack load failed: %s\n", packLoadErrorLabel(_lastError));
    listDirectory(_root);
    unload();
    return false;
  }
  if (!loadConfig()) {
    _lastError = PackLoadError::Config;
    Serial.printf("Pack load failed: %s\n", packLoadErrorLabel(_lastError));
    unload();
    return false;
  }
  if (!loadAnimations()) {
    _lastError = PackLoadError::Animations;
    Serial.printf("Pack load failed: %s\n", packLoadErrorLabel(_lastError));
    listDirectory(_root + "/animations");
    unload();
    return false;
  }

  _loaded = true;
  _lastError = PackLoadError::None;
  return true;
}

bool PackLoader::loadManifest() {
  std::string path = _root + "/manifest.json";
  std::string text = readTextFile(path);
  if (text.empty()) {
    Serial.printf("Missing manifest: %s\n", path.c_str());
    return false;
  }

  StaticJsonDocument<8192> doc;
  if (deserializeJson(doc, text)) {
    Serial.println("manifest parse error");
    return false;
  }

  _info.packId = doc["pack_id"] | "";
  _info.uuid = doc["uuid"] | "";
  _info.profile = doc["profile"] | "";
  if (doc["version"].is<JsonObject>()) {
    _info.version.major = doc["version"]["major"] | 0;
    _info.version.minor = doc["version"]["minor"] | 0;
    _info.version.patch = doc["version"]["patch"] | 0;
  }
  if (doc["display"].is<JsonObject>()) {
    _info.displayWidth = doc["display"]["width"] | 170;
    _info.displayHeight = doc["display"]["height"] | 320;
  }

  _sprites.clear();
  JsonArray sprites = doc["sprites"].as<JsonArray>();
  for (JsonVariant v : sprites) {
    JsonObject sprite = v.as<JsonObject>();
    SpriteMeta meta;
    meta.id = sprite["id"] | "";
    meta.file = sprite["file"] | "";
    meta.width = sprite["width"] | 0;
    meta.height = sprite["height"] | 0;
    meta.bytes = sprite["bytes"] | 0;
    if (!meta.id.empty()) {
      _sprites.push_back(meta);
    }
  }
  Serial.printf("manifest: %u sprites\n", static_cast<unsigned>(_sprites.size()));
  return !_sprites.empty();
}

bool PackLoader::loadConfig() {
  std::string path = _root + "/config.json";
  std::string text = readTextFile(path);
  if (text.empty()) {
    return true;
  }

  StaticJsonDocument<2048> doc;
  if (deserializeJson(doc, text)) {
    Serial.println("config parse error");
    return false;
  }

  const char *defaultBg = doc["display"]["default_background"] | "office";
  if (doc["backgrounds"][defaultBg].is<JsonObject>()) {
    _defaultBgSprite = doc["backgrounds"][defaultBg]["sprite"] | "bg_office";
  } else {
    _defaultBgSprite = "bg_office";
  }

  if (doc["anchors"]["head"].is<JsonObject>()) {
    _anchorX = doc["anchors"]["head"]["x"] | 85;
    _anchorY = doc["anchors"]["head"]["y"] | 80;
  }

  _expressions.clear();
  _defaultExpressionSprite = "face_neutral";
  JsonObject exprMap = doc["expressions"].as<JsonObject>();
  if (!exprMap.isNull()) {
    for (JsonPair kv : exprMap) {
      const char *sprite = kv.value().as<const char *>();
      if (sprite && sprite[0]) {
        _expressions.emplace_back(kv.key().c_str(), sprite);
      }
    }
  }
  return true;
}

const char *PackLoader::expressionForEmotion(const char *emotion) const {
  if (!emotion || !emotion[0]) {
    return _defaultExpressionSprite.c_str();
  }
  for (const auto &pair : _expressions) {
    if (pair.first == emotion) {
      return pair.second.c_str();
    }
  }
  if (strcmp(emotion, "excited") == 0) {
    for (const auto &pair : _expressions) {
      if (pair.first == "happy") {
        return pair.second.c_str();
      }
    }
  }
  if (strcmp(emotion, "curious") == 0) {
    for (const auto &pair : _expressions) {
      if (pair.first == "thinking") {
        return pair.second.c_str();
      }
    }
  }
  return _defaultExpressionSprite.c_str();
}

bool PackLoader::parseAnimationJson(const std::string &text, const char *fallbackId) {
  StaticJsonDocument<4096> doc;
  if (deserializeJson(doc, text)) {
    return false;
  }
  AnimationClip clip;
  clip.id = doc["id"] | fallbackId;
  clip.loop = doc["loop"] | true;
  JsonArray frames = doc["frames"].as<JsonArray>();
  for (JsonVariant v : frames) {
    JsonObject frame = v.as<JsonObject>();
    ClipFrame cf;
    cf.spriteId = frame["sprite"] | "";
    cf.durationMs = frame["duration_ms"] | 500;
    if (!cf.spriteId.empty()) {
      clip.frames.push_back(cf);
    }
  }
  if (clip.frames.empty()) {
    return false;
  }
  for (const auto &existing : _animations) {
    if (existing.id == clip.id) {
      return true;
    }
  }
  _animations.push_back(clip);
  return true;
}

bool PackLoader::loadAnimationById(const std::string &animId) {
  for (const auto &existing : _animations) {
    if (existing.id == animId) {
      return true;
    }
  }
  std::string path = _root + "/animations/" + animId + ".json";
  std::string text = readTextFile(path);
  if (text.empty()) {
    Serial.printf("Missing animation file: %s\n", path.c_str());
    return false;
  }
  if (!parseAnimationJson(text, animId.c_str())) {
    Serial.printf("Animation parse failed: %s\n", path.c_str());
    return false;
  }
  return true;
}

bool PackLoader::loadAnimations() {
  _animations.clear();
  _graphText.clear();

  std::string graphPath = _root + "/animation_graph.json";
  _graphText = readTextFile(graphPath);
  if (!_graphText.empty()) {
    StaticJsonDocument<4096> graphDoc;
    if (!deserializeJson(graphDoc, _graphText)) {
      JsonObject states = graphDoc["states"].as<JsonObject>();
      for (JsonPair kv : states) {
        const char *animId = kv.value()["animation"] | "";
        if (animId[0]) {
          loadAnimationById(animId);
        }
      }
    }
  }

  std::string animDir = _root + "/animations";
  File dir = LittleFS.open(animDir.c_str(), "r");
  if (!dir || !dir.isDirectory()) {
    dir = LittleFS.open((animDir + "/").c_str(), "r");
  }
  if (dir && dir.isDirectory()) {
    File entry = dir.openNextFile();
    while (entry) {
      std::string name = entry.name();
      if (name.length() >= 5 && name.compare(name.length() - 5, 5, ".json") == 0) {
        std::string fullPath = (name.length() > 0 && name[0] == '/') ? name : (animDir + "/" + name);
        std::string text = readTextFile(fullPath);
        if (!text.empty()) {
          std::string stem = name;
          if (stem[0] == '/') {
            size_t slash = stem.find_last_of('/');
            if (slash != std::string::npos) {
              stem = stem.substr(slash + 1);
            }
          }
          if (stem.length() > 5) {
            stem = stem.substr(0, stem.length() - 5);
          }
          parseAnimationJson(text, stem.c_str());
        }
      }
      entry.close();
      entry = dir.openNextFile();
    }
    dir.close();
  }

  if (!_animations.empty()) {
    Serial.printf("animations: %u clips\n", static_cast<unsigned>(_animations.size()));
    return true;
  }

  loadAnimationById("idle");
  loadAnimationById("coding");
  if (!_animations.empty()) {
    Serial.printf("animations: %u clips (fallback)\n", static_cast<unsigned>(_animations.size()));
    return true;
  }

  Serial.printf("Missing animations under: %s\n", animDir.c_str());
  return false;
}

const SpriteMeta *PackLoader::findSprite(const char *id) const {
  if (!id) {
    return nullptr;
  }
  for (const auto &s : _sprites) {
    if (s.id == id) {
      return &s;
    }
  }
  return nullptr;
}

const AnimationClip *PackLoader::findAnimation(const char *id) const {
  if (!id) {
    return nullptr;
  }
  for (const auto &a : _animations) {
    if (a.id == id) {
      return &a;
    }
  }
  return nullptr;
}

bool PackLoader::readSpritePixels(const SpriteMeta *meta, uint16_t *buffer, size_t bufferBytes) const {
  if (!meta || !buffer) {
    return false;
  }
  size_t needed = static_cast<size_t>(meta->width) * static_cast<size_t>(meta->height) * 2;
  if (bufferBytes < needed) {
    return false;
  }

  std::string path = _root + "/" + meta->file;
  File f = LittleFS.open(path.c_str(), "r");
  if (!f && !path.empty() && path[0] == '/') {
    f = LittleFS.open(path.c_str() + 1, "r");
  }
  if (!f) {
    Serial.printf("Sprite open failed: %s\n", path.c_str());
    return false;
  }
  size_t read = f.read(reinterpret_cast<uint8_t *>(buffer), needed);
  f.close();
  return read == needed;
}
