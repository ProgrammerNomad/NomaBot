#include "animation_graph.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

bool AnimationGraph::parseGraphJson(const std::string &text) {
  _states.clear();

  StaticJsonDocument<4096> doc;
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
    JsonArray transitions = stateObj["transitions"].as<JsonArray>();
    for (JsonVariant tv : transitions) {
      JsonObject tr = tv.as<JsonObject>();
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

bool AnimationGraph::loadFromText(const std::string &text) {
  return parseGraphJson(text);
}

bool AnimationGraph::loadFromPack(const std::string &rootPath, const char *graphFile) {
  std::string path = rootPath + "/" + graphFile;
  File f = LittleFS.open(path.c_str(), "r");
  if (!f && !path.empty() && path[0] == '/') {
    f = LittleFS.open(path.c_str() + 1, "r");
  }
  if (!f) {
    return false;
  }

  std::string text;
  while (f.available()) {
    text += static_cast<char>(f.read());
  }
  f.close();
  return parseGraphJson(text);
}

void AnimationGraph::applyDefaults() {
  _states.clear();
  _defaultState = "idle";
  _currentState = "idle";
  GraphState idle;
  idle.name = "idle";
  idle.animation = "idle";
  GraphState coding;
  coding.name = "coding";
  coding.animation = "coding";
  _states.push_back(idle);
  _states.push_back(coding);
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
