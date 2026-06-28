#pragma once

#include <string>
#include <vector>

struct GraphTransition {
  std::string toState;
  std::string onEvent;
  int blendMs = 0;
};

struct GraphState {
  std::string name;
  std::string animation;
  std::vector<GraphTransition> transitions;
};

class AnimationGraph {
public:
  bool loadFromPack(const std::string &rootPath, const char *graphFile = "animation_graph.json");
  bool loadFromText(const std::string &text);
  void applyDefaults();
  void setState(const char *state);
  const char *currentState() const { return _currentState.c_str(); }
  const char *animationForState(const char *state) const;
  const char *currentAnimation() const;

private:
  bool parseGraphJson(const std::string &text);

  std::vector<GraphState> _states;
  std::string _defaultState = "idle";
  std::string _currentState = "idle";
};
