#pragma once

#include "render/render_state.h"

class DirtyTracker {
public:
  DirtyFlags collectDirtyFlags(const RenderState &next);
  void invalidate(DirtyFlags flags = DirtyFull);
  void forceDirty(DirtyFlags flags);
  void notePending(DirtyFlags flags);
  void commitRendered(const RenderState &state);
  const RenderState &lastRendered() const { return _last; }
  bool initialized() const { return _initialized; }

private:
  RenderState _last{};
  bool _initialized = false;
  bool _forceFull = false;
  DirtyFlags _pending = DirtyNone;
  DirtyFlags _forced = DirtyNone;

  static bool strChanged(const char *a, const char *b);
  DirtyFlags computeDiff(const RenderState &next) const;
};
