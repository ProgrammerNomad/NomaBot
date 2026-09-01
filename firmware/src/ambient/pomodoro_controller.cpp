#include "ambient/pomodoro_controller.h"

void PomodoroController::begin() {
  _state = PomodoroState::Idle;
  _totalSec = 25 * 60;
  _remainingSec = _totalSec;
  _lastTickMs = 0;
  _completedFlag = false;
}

void PomodoroController::tick(unsigned long nowMs) {
  if (_state != PomodoroState::Running) {
    _lastTickMs = nowMs;
    return;
  }
  if (_lastTickMs == 0) {
    _lastTickMs = nowMs;
    return;
  }
  unsigned long elapsed = (nowMs - _lastTickMs) / 1000UL;
  if (elapsed == 0) {
    return;
  }
  _lastTickMs = nowMs;
  if (elapsed >= _remainingSec) {
    _remainingSec = 0;
    _state = PomodoroState::Complete;
    _completedFlag = true;
    return;
  }
  _remainingSec -= elapsed;
}

void PomodoroController::toggleStartPause(unsigned long nowMs) {
  if (_state == PomodoroState::Idle || _state == PomodoroState::Complete) {
    _remainingSec = _totalSec;
    _state = PomodoroState::Running;
    _lastTickMs = nowMs;
    _completedFlag = false;
    return;
  }
  if (_state == PomodoroState::Running) {
    _state = PomodoroState::Paused;
    return;
  }
  if (_state == PomodoroState::Paused) {
    _state = PomodoroState::Running;
    _lastTickMs = nowMs;
  }
}

void PomodoroController::reset() {
  _state = PomodoroState::Idle;
  _remainingSec = _totalSec;
  _completedFlag = false;
}

bool PomodoroController::consumeCompleted() {
  if (!_completedFlag) {
    return false;
  }
  _completedFlag = false;
  return true;
}
