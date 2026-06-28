#include "overlay_manager.h"

#include <string.h>

void OverlayManager::push(const char *id, const char *text, int priority,
                          unsigned long durationMs, unsigned long nowMs) {
  if (!text || !text[0]) {
    return;
  }
  const char *overlayId = (id && id[0]) ? id : "anonymous";
  if (_hasActive && _active.id == overlayId) {
    _active.text = text;
    _active.priority = priority;
    _active.expiresAtMs = durationMs > 0 ? nowMs + durationMs : 0;
    return;
  }
  if (_hasActive && priority < _active.priority) {
    return;
  }
  _active.id = overlayId;
  _active.text = text;
  _active.priority = priority;
  _active.expiresAtMs = durationMs > 0 ? nowMs + durationMs : 0;
  _hasActive = true;
}

bool OverlayManager::cancel(const char *id) {
  if (!id || !id[0] || !_hasActive) {
    return false;
  }
  if (_active.id != id) {
    return false;
  }
  _hasActive = false;
  _active = QueuedOverlay{};
  return true;
}

bool OverlayManager::tick(unsigned long nowMs) {
  if (!_hasActive) {
    return false;
  }
  if (_active.expiresAtMs > 0 && nowMs >= _active.expiresAtMs) {
    _hasActive = false;
    _active = QueuedOverlay{};
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

const char *OverlayManager::activeId() const {
  if (!_hasActive || _active.id.empty()) {
    return "";
  }
  return _active.id.c_str();
}

int OverlayManager::queueDepth() const { return _hasActive ? 1 : 0; }
