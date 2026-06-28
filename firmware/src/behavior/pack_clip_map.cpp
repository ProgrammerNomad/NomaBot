#include "pack_clip_map.h"

#include <ArduinoJson.h>
#include <string.h>

void PackClipMap::clear() { _entries.clear(); }

void PackClipMap::addEntry(const char *behaviorId, const char *clipId) {
  if (!behaviorId || !behaviorId[0] || !clipId || !clipId[0]) {
    return;
  }
  for (const auto &entry : _entries) {
    if (entry.behaviorId == behaviorId) {
      return;
    }
  }
  _entries.push_back({behaviorId, clipId});
}

const PackClipMap::Entry *PackClipMap::findEntry(const char *behaviorId) const {
  if (!behaviorId) {
    return nullptr;
  }
  for (const auto &entry : _entries) {
    if (entry.behaviorId == behaviorId) {
      return &entry;
    }
  }
  return nullptr;
}

void PackClipMap::loadBehaviorArray(JsonArray behaviors) {
  for (JsonVariant v : behaviors) {
    JsonObject behavior = v.as<JsonObject>();
    const char *id = behavior["id"] | "";
    const char *clip = behavior["clip"] | "";
    if (id[0] && clip[0]) {
      addEntry(id, clip);
    }
  }
}

void PackClipMap::loadActivities(JsonObject activities) {
  for (JsonPair activity : activities) {
    JsonObject activityObj = activity.value().as<JsonObject>();
    JsonArray behaviors = activityObj["behaviors"].as<JsonArray>();
    if (!behaviors.isNull()) {
      loadBehaviorArray(behaviors);
    }
  }
}

void PackClipMap::loadEmotions(JsonObject emotions) {
  for (JsonPair emotion : emotions) {
    JsonObject emotionObj = emotion.value().as<JsonObject>();
    for (JsonPair activity : emotionObj) {
      JsonObject activityObj = activity.value().as<JsonObject>();
      JsonArray behaviors = activityObj["behaviors"].as<JsonArray>();
      if (!behaviors.isNull()) {
        loadBehaviorArray(behaviors);
      }
    }
  }
}

void PackClipMap::loadBehaviorClips(JsonObject behaviorClips) {
  for (JsonPair kv : behaviorClips) {
    const char *clip = kv.value().as<const char *>();
    if (clip && clip[0]) {
      addEntry(kv.key().c_str(), clip);
    }
  }
}

bool PackClipMap::loadFromJsonText(const std::string &text) {
  clear();
  if (text.empty()) {
    return false;
  }

  DynamicJsonDocument doc(12288);
  if (deserializeJson(doc, text)) {
    return false;
  }

  JsonObject activities = doc["activities"].as<JsonObject>();
  if (!activities.isNull()) {
    loadActivities(activities);
  }

  JsonObject emotions = doc["emotions"].as<JsonObject>();
  if (!emotions.isNull()) {
    loadEmotions(emotions);
  }

  JsonObject behaviorClips = doc["behavior_clips"].as<JsonObject>();
  if (!behaviorClips.isNull()) {
    loadBehaviorClips(behaviorClips);
  }

  return !_entries.empty();
}

const char *PackClipMap::clipForBehavior(const char *behaviorId) const {
  const Entry *entry = findEntry(behaviorId);
  if (entry && !entry->clipId.empty()) {
    return entry->clipId.c_str();
  }
  return nullptr;
}
