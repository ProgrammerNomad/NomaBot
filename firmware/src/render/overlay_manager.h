#pragma once

#include <string>

struct QueuedOverlay {
  std::string text;
  int priority = 0;
  unsigned long expiresAtMs = 0;
};

class OverlayManager {
public:
  void push(const char *text, int priority, unsigned long durationMs, unsigned long nowMs);
  bool tick(unsigned long nowMs);
  const char *activeText() const;

private:
  QueuedOverlay _active;
  bool _hasActive = false;
};
