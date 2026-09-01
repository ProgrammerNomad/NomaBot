#pragma once

class WifiService {
public:
  void begin();
  void tick();
  bool connected() const { return _connected; }
  bool reconnecting() const { return _reconnecting; }
  int rssi() const;

private:
  bool _connected = false;
  bool _reconnecting = false;
  unsigned long _lastAttemptMs = 0;
  unsigned long _backoffMs = 10000;
  unsigned int _attemptCount = 0;
};
