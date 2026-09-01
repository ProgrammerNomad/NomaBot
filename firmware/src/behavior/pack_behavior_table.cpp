#include "pack_behavior_table.h"

#include <string.h>

void PackBehaviorTable::clear() { _activities.clear(); }

void PackBehaviorTable::loadBehaviorArray(JsonArray behaviors,
                                          std::vector<PackBehaviorEntry> &out) {
  for (JsonVariant v : behaviors) {
    JsonObject behavior = v.as<JsonObject>();
    const char *id = behavior["id"] | "";
    const char *clip = behavior["clip"] | "";
    if (!id[0] || !clip[0]) {
      continue;
    }
    PackBehaviorEntry entry;
    entry.id = id;
    entry.clip = clip;
    entry.label = behavior["label"] | "";
    entry.weight = static_cast<uint8_t>(behavior["weight"] | 1);
    JsonObject duration = behavior["duration"].as<JsonObject>();
    if (!duration.isNull()) {
      entry.durationMinSec = static_cast<uint16_t>(duration["min"] | entry.durationMinSec);
      entry.durationMaxSec = static_cast<uint16_t>(duration["max"] | entry.durationMaxSec);
    }
    if (entry.durationMaxSec < entry.durationMinSec) {
      entry.durationMaxSec = entry.durationMinSec;
    }
    if (entry.weight < 1) {
      entry.weight = 1;
    }
    out.push_back(entry);
  }
}

void PackBehaviorTable::loadActivities(JsonObject activities) {
  for (JsonPair activity : activities) {
    JsonObject activityObj = activity.value().as<JsonObject>();
    JsonArray behaviors = activityObj["behaviors"].as<JsonArray>();
    if (behaviors.isNull()) {
      continue;
    }
    ActivityTable table;
    table.activityId = activity.key().c_str();
    loadBehaviorArray(behaviors, table.behaviors);
    if (!table.behaviors.empty()) {
      _activities.push_back(table);
    }
  }
}

bool PackBehaviorTable::loadFromJsonText(const std::string &text) {
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

  return !_activities.empty();
}

const std::vector<PackBehaviorEntry> *PackBehaviorTable::behaviorsForActivity(
    const char *activityId) const {
  if (!activityId) {
    return nullptr;
  }
  for (const auto &table : _activities) {
    if (table.activityId == activityId) {
      return &table.behaviors;
    }
  }
  return nullptr;
}

const PackBehaviorEntry *PackBehaviorTable::findBehavior(const char *behaviorId) const {
  if (!behaviorId) {
    return nullptr;
  }
  for (const auto &table : _activities) {
    for (const auto &entry : table.behaviors) {
      if (entry.id == behaviorId) {
        return &entry;
      }
    }
  }
  return nullptr;
}
