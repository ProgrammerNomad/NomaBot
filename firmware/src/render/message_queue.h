#pragma once

#include <string>

struct QueuedMessage {
  std::string text;
  int priority = 0;
  unsigned long expiresAtMs = 0;
};

class MessageQueue {
public:
  void push(const char *text, int priority, unsigned long durationMs, unsigned long nowMs);
  void tick(unsigned long nowMs);
  const char *activeText() const;

private:
  QueuedMessage _active;
  bool _hasActive = false;
};
