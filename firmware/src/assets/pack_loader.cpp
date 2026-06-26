#include "pack_loader.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

bool PackLoader::mountFilesystem() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
    return false;
  }
  return true;
}

std::string PackLoader::readTextFile(const std::string &path) const {
  File f = LittleFS.open(path.c_str(), "r");
  if (!f) {
    return {};
  }
  std::string content;
  while (f.available()) {
    content += static_cast<char>(f.read());
  }
  f.close();
  return content;
}

void PackLoader::unload() {
  _loaded = false;
  _characterId.clear();
  _root.clear();
  _sprites.clear();
  _animations.clear();
  _defaultBgSprite.clear();
}

bool PackLoader::load(const char *characterId) {
  unload();
  _characterId = characterId;
  _root = std::string("/characters/") + characterId;

  if (!loadManifest() || !loadConfig() || !loadAnimations()) {
    unload();
    return false;
  }
  _loaded = true;
  return true;
}

bool PackLoader::loadManifest() {
  std::string path = _root + "/manifest.json";
  std::string text = readTextFile(path);
  if (text.empty()) {
    Serial.printf("Missing manifest: %s\n", path.c_str());
    return false;
  }

  JsonDocument doc;
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
  for (JsonObject sprite : doc["sprites"].as<JsonArray>()) {
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
  return !_sprites.empty();
}

bool PackLoader::loadConfig() {
  std::string path = _root + "/config.json";
  std::string text = readTextFile(path);
  if (text.empty()) {
    return true;
  }

  JsonDocument doc;
  if (deserializeJson(doc, text)) {
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
  return true;
}

bool PackLoader::loadAnimations() {
  std::string animDir = _root + "/animations";
  File dir = LittleFS.open(animDir.c_str());
  if (!dir || !dir.isDirectory()) {
    Serial.printf("Missing animations dir: %s\n", animDir.c_str());
    return false;
  }

  _animations.clear();
  File entry = dir.openNextFile();
  while (entry) {
    std::string name = entry.name();
    if (name.length() >= 5 && name.compare(name.length() - 5, 5, ".json") == 0) {
      std::string fullPath = animDir + "/" + name;
      std::string text = readTextFile(fullPath);
      JsonDocument doc;
      if (!deserializeJson(doc, text)) {
        AnimationClip clip;
        clip.id = doc["id"] | name.c_str();
        clip.loop = doc["loop"] | true;
        for (JsonObject frame : doc["frames"].as<JsonArray>()) {
          ClipFrame cf;
          cf.spriteId = frame["sprite"] | "";
          cf.durationMs = frame["duration_ms"] | 500;
          if (!cf.spriteId.empty()) {
            clip.frames.push_back(cf);
          }
        }
        if (!clip.frames.empty()) {
          _animations.push_back(clip);
        }
      }
    }
    entry.close();
    entry = dir.openNextFile();
  }
  dir.close();
  return !_animations.empty();
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
  if (!f) {
    Serial.printf("Sprite open failed: %s\n", path.c_str());
    return false;
  }
  size_t read = f.read(reinterpret_cast<uint8_t *>(buffer), needed);
  f.close();
  return read == needed;
}
