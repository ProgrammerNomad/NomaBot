#include "animation_graph.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

bool AnimationGraph::loadFromPack(const std::string &rootPath, const char *graphFile) {
  _states.clear();
  std::string path = rootPath + "/" + graphFile;
  File f = LittleFS.open(path.c_str(), "r");
  if (!f) {
    return false;
  }

  std::string text;
  while (f.available()) {
    text += static_cast<char>(f.read());
  }
  f.close();

  JsonDocument doc;
  if (deserializeJson(doc, text)) {
    return false;
  }

  _defaultState = doc["default_state"] | "idle";
  _currentState = _defaultState;

  JsonObject states = doc["states"].as<JsonObject>();
  for (JsonPair kv : states) {
    GraphState gs;
    gs.name = kv.key().c_str();
    JsonObject stateObj = kv.value().as<JsonObject>();
    gs.animation = stateObj["animation"] | gs.name.c_str();
    for (JsonObject tr : stateObj["transitions"].as<JsonArray>()) {
      GraphTransition gt;
      gt.toState = tr["to"] | "";
      gt.onEvent = tr["on"] | "";
      gt.blendMs = tr["blend_ms"] | 0;
      if (!gt.toState.empty()) {
        gs.transitions.push_back(gt);
      }
    }
    _states.push_back(gs);
  }
  return !_states.empty();
}

void AnimationGraph::setState(const char *state) {
  if (state && *state) {
    _currentState = state;
  }
}

const char *AnimationGraph::animationForState(const char *state) const {
  if (!state) {
    return "idle";
  }
  for (const auto &s : _states) {
    if (s.name == state) {
      return s.animation.c_str();
    }
  }
  return state;
}

const char *AnimationGraph::currentAnimation() const {
  return animationForState(_currentState.c_str());
}
