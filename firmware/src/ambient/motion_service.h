#pragma once

class MotionService {
public:
  void begin();
  void tick();
  int tiltX() const { return _tiltX; }
  int tiltY() const { return _tiltY; }

private:
  int _tiltX = 0;
  int _tiltY = 0;
};
