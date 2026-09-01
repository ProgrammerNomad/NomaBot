#pragma once

#include <ArduinoJson.h>
#include <string>
#include <vector>

struct PackBehaviorEntry {
  std::string id;
  std::string label;
  std::string clip;
  uint8_t weight = 1;
  uint16_t durationMinSec = 3;
  uint16_t durationMaxSec = 8;
};

class PackBehaviorTable {
public:
  void clear();
  bool loadFromJsonText(const std::string &text);
  const std::vector<PackBehaviorEntry> *behaviorsForActivity(const char *activityId) const;
  const PackBehaviorEntry *findBehavior(const char *behaviorId) const;
  bool hasActivities() const { return !_activities.empty(); }

private:
  struct ActivityTable {
    std::string activityId;
    std::vector<PackBehaviorEntry> behaviors;
  };

  std::vector<ActivityTable> _activities;

  void loadBehaviorArray(JsonArray behaviors, std::vector<PackBehaviorEntry> &out);
  void loadActivities(JsonObject activities);
};
