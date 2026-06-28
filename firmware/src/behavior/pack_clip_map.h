#pragma once

#include <ArduinoJson.h>
#include <string>
#include <vector>

class PackClipMap {
public:
  void clear();
  bool loadFromJsonText(const std::string &text);
  const char *clipForBehavior(const char *behaviorId) const;

private:
  struct Entry {
    std::string behaviorId;
    std::string clipId;
  };

  std::vector<Entry> _entries;

  void addEntry(const char *behaviorId, const char *clipId);
  const Entry *findEntry(const char *behaviorId) const;
  void loadBehaviorArray(JsonArray behaviors);
  void loadActivities(JsonObject activities);
  void loadEmotions(JsonObject emotions);
  void loadBehaviorClips(JsonObject behaviorClips);
};
