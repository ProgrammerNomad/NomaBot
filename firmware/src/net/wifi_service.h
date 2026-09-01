#pragma once

class WifiService {
public:
  void begin();
  void tick();
  bool connected() const { return _connected; }

private:
  bool _connected = false;
  unsigned long _lastAttemptMs = 0;
};
