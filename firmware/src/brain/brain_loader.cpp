#include "brain_loader.h"

#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

#include "brain.h"

namespace {

bool readJsonFile(const char *jsonPath, std::string &text) {
  if (!jsonPath) {
    return false;
  }
  File f = LittleFS.open(jsonPath, "r");
  if (!f && jsonPath[0] == '/') {
    f = LittleFS.open(jsonPath + 1, "r");
  }
  if (!f) {
    return false;
  }
  text.clear();
  while (f.available()) {
    text += static_cast<char>(f.read());
  }
  f.close();
  return !text.empty();
}

}  // namespace

bool brainLoadFromJson(Brain &brain, const char *jsonPath) {
  std::string text;
  if (!readJsonFile(jsonPath, text)) {
    return false;
  }

  DynamicJsonDocument doc(12288);
  if (deserializeJson(doc, text)) {
    return false;
  }

  PersonalityTraits traits;
  JsonObject personality = doc["personality"].as<JsonObject>();
  if (!personality.isNull()) {
    traits.energy = personality["energy"] | traits.energy;
    traits.curiosity = personality["curiosity"] | traits.curiosity;
    brain.setPersonality(traits);
  }

  brain.loadClipMapFromJsonText(text);
  brain.loadPackBehaviorsFromJsonText(text);
  return true;
}
