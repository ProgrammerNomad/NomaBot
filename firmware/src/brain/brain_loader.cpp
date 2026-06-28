#include "brain_loader.h"

#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

#include "brain.h"

bool brainLoadFromJson(Brain &brain, const char *jsonPath) {
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
  std::string text;
  while (f.available()) {
    text += static_cast<char>(f.read());
  }
  f.close();

  StaticJsonDocument<2048> doc;
  if (deserializeJson(doc, text)) {
    return false;
  }

  PersonalityTraits traits;
  JsonObject personality = doc["personality"].as<JsonObject>();
  if (!personality.isNull()) {
    traits.energy = personality["energy"] | traits.energy;
    traits.coffeeLove = personality["coffee_love"] | traits.coffeeLove;
    traits.curiosity = personality["curiosity"] | traits.curiosity;
    traits.sleepiness = personality["sleepiness"] | traits.sleepiness;
    traits.optimism = personality["optimism"] | traits.optimism;
    traits.patience = personality["patience"] | traits.patience;
    traits.playfulness = personality["playfulness"] | traits.playfulness;
    brain.setPersonality(traits);
  }
  return true;
}
