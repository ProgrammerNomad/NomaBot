#pragma once

class ProvisioningService {
public:
  bool needsSetup() const;
  bool startPortal();
  void tick();
  bool active() const { return _active; }

private:
  bool _active = false;
};
