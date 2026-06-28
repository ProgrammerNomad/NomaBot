#pragma once

#include <ArduinoJson.h>

class CommandHistory {
public:
  static constexpr size_t kCapacity = 10;

  void record(const char *cmd, const char *detail, unsigned long nowMs);
  void appendToJson(JsonArray arr) const;

private:
  struct Entry {
    unsigned long tMs = 0;
    char cmd[24]{};
    char detail[32]{};
  };

  Entry _entries[kCapacity]{};
  size_t _count = 0;
  size_t _next = 0;
};

extern CommandHistory gCommandHistory;
