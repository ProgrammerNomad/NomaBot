#include "clip_player.h"

void ClipPlayer::setClip(const AnimationClip *clip) {
  _clip = clip;
  reset();
}

void ClipPlayer::reset() {
  _frameIndex = 0;
  _frameStartMs = 0;
  _lastTickMs = 0;
}

const char *ClipPlayer::currentSpriteId() const {
  if (!_clip || _clip->frames.empty()) {
    return "";
  }
  if (_frameIndex < 0 || _frameIndex >= static_cast<int>(_clip->frames.size())) {
    return _clip->frames[0].spriteId.c_str();
  }
  return _clip->frames[_frameIndex].spriteId.c_str();
}

void ClipPlayer::tick(unsigned long nowMs) {
  if (!_clip || _clip->frames.empty()) {
    return;
  }
  if (_lastTickMs == 0) {
    _lastTickMs = nowMs;
    _frameStartMs = nowMs;
    return;
  }
  _lastTickMs = nowMs;

  const ClipFrame &frame = _clip->frames[_frameIndex];
  if (nowMs - _frameStartMs < static_cast<unsigned long>(frame.durationMs)) {
    return;
  }

  _frameStartMs = nowMs;
  _frameIndex++;
  if (_frameIndex >= static_cast<int>(_clip->frames.size())) {
    if (_clip->loop) {
      _frameIndex = 0;
    } else {
      _frameIndex = static_cast<int>(_clip->frames.size()) - 1;
    }
  }
}
