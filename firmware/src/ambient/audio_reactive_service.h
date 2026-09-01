#pragma once

class AudioReactiveService {
public:
  void begin();
  void tick();
  float volumeLevel() const { return _volume; }

private:
  float _volume = 0.0f;
};
