#include "command_history.h"

#include <string.h>

CommandHistory gCommandHistory;

void CommandHistory::record(const char *cmd, const char *detail, unsigned long nowMs) {
  Entry &e = _entries[_next];
  e.tMs = nowMs;
  if (cmd) {
    strncpy(e.cmd, cmd, sizeof(e.cmd) - 1);
    e.cmd[sizeof(e.cmd) - 1] = '\0';
  } else {
    e.cmd[0] = '\0';
  }
  if (detail) {
    strncpy(e.detail, detail, sizeof(e.detail) - 1);
    e.detail[sizeof(e.detail) - 1] = '\0';
  } else {
    e.detail[0] = '\0';
  }
  _next = (_next + 1) % kCapacity;
  if (_count < kCapacity) {
    _count++;
  }
}

void CommandHistory::appendToJson(JsonArray arr) const {
  if (_count == 0) {
    return;
  }
  size_t start = _count < kCapacity ? 0 : _next;
  for (size_t i = 0; i < _count; ++i) {
    size_t idx = (start + i) % kCapacity;
    JsonObject row = arr.add<JsonObject>();
    row["t_ms"] = _entries[idx].tMs;
    row["cmd"] = _entries[idx].cmd;
    row["detail"] = _entries[idx].detail;
  }
}
