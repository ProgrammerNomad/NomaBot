#pragma once

class BleCompanionService {
public:
  void begin();
  void tick();
  void notifyAlert(const char *message);
};
