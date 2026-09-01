#pragma once

struct ServiceStatus {
  bool wifiConnected = false;
  bool wifiReconnecting = false;
  bool clockValid = false;
  bool weatherOk = false;
  bool weatherStale = false;
};

ServiceStatus collectServiceStatus();
