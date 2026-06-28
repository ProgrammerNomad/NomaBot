#pragma once

#include <string>

struct QueuedOverlay {
  std::string id;
  std::string text;
  int priority = 0;
  unsigned long expiresAtMs = 0;
};

class OverlayManager {
public:
  void push(const char *id, const char *text, int priority, unsigned long durationMs,
            unsigned long nowMs);
  bool cancel(const char *id);
  bool tick(unsigned long nowMs);
  const char *activeText() const;
  const char *activeId() const;
  int queueDepth() const;

private:
  QueuedOverlay _active;
  bool _hasActive = false;
};
