#include "overlay_manager.h"

#include <string.h>

void OverlayManager::push(const char *text, int priority, unsigned long durationMs,
                          unsigned long nowMs) {
  if (!text || !text[0]) {
    return;
  }
  if (_hasActive && priority < _active.priority) {
    return;
  }
  _active.text = text;
  _active.priority = priority;
  _active.expiresAtMs = durationMs > 0 ? nowMs + durationMs : 0;
  _hasActive = true;
}

bool OverlayManager::tick(unsigned long nowMs) {
  if (!_hasActive) {
    return false;
  }
  if (_active.expiresAtMs > 0 && nowMs >= _active.expiresAtMs) {
    _hasActive = false;
    _active.text.clear();
    _active.priority = 0;
    _active.expiresAtMs = 0;
    return true;
  }
  return false;
}

const char *OverlayManager::activeText() const {
  if (!_hasActive || _active.text.empty()) {
    return "";
  }
  return _active.text.c_str();
}
