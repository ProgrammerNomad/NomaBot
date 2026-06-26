#pragma once

#include <ArduinoJson.h>
#include <functional>
#include <map>
#include <string>

struct ProtocolResponse {
  std::string line;
  bool ok = true;

  ProtocolResponse() = default;
  ProtocolResponse(std::string line_, bool ok_ = true) : line(std::move(line_)), ok(ok_) {}
};

class ProtocolHandler {
public:
  using CommandHandler = std::function<ProtocolResponse(const std::string &id, JsonObject params)>;

  void registerCommand(const char *name, CommandHandler handler) {
    _handlers[name] = handler;
  }

  void processLine(const std::string &line, std::function<void(const std::string &)> emit);

private:
  std::map<std::string, CommandHandler> _handlers;

  std::string buildResponse(const std::string &id, const char *cmd, bool ok,
                            JsonDocument &data, JsonDocument *error = nullptr);
};
