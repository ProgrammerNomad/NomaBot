#pragma once

#include <string>

class AccessoryManager {
public:
  void clear() { _accessoryId.clear(); }
  void setAccessory(const char *id) { _accessoryId = id ? id : ""; }
  const char *accessoryId() const { return _accessoryId.c_str(); }

private:
  std::string _accessoryId;
};
