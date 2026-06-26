#include "handler.h"
#include <Arduino.h>

void ProtocolHandler::processLine(const std::string &line,
                                  std::function<void(const std::string &)> emit) {
  if (line.empty() || line.length() > 16384) {
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, line);
  if (err) {
    JsonDocument errDoc;
    errDoc["code"] = "parse_error";
    errDoc["message"] = err.c_str();
    JsonDocument empty;
    emit(buildResponse("unknown", "", false, empty, &errDoc));
    return;
  }

  int v = doc["v"] | 0;
  if (v < 1) {
    JsonDocument errDoc;
    errDoc["code"] = "unsupported_version";
    errDoc["message"] = "missing or invalid v";
    std::string id = doc["id"] | "unknown";
    JsonDocument empty;
    emit(buildResponse(id, "", false, empty, &errDoc));
    return;
  }

  std::string id = doc["id"].as<std::string>();
  std::string type = doc["type"] | "";
  std::string cmd = doc["cmd"] | "";

  if (type != "command") {
    return;
  }

  auto it = _handlers.find(cmd);
  if (it == _handlers.end()) {
    JsonDocument errDoc;
    errDoc["code"] = "unknown_command";
    errDoc["message"] = ("Unknown: " + cmd).c_str();
    JsonDocument empty;
    emit(buildResponse(id, cmd.c_str(), false, empty, &errDoc));
    return;
  }

  JsonObject params = doc["params"].as<JsonObject>();
  ProtocolResponse resp = it->second(id, params);
  emit(resp.line);
}

std::string ProtocolHandler::buildResponse(const std::string &id, const char *cmd, bool ok,
                                           JsonDocument &data, JsonDocument *error) {
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = ok ? "response" : "error";
  if (cmd[0]) doc["cmd"] = cmd;
  doc["ok"] = ok;
  if (!data.isNull()) doc["data"] = data;
  if (error && !error->isNull()) doc["error"] = *error;

  std::string out;
  serializeJson(doc, out);
  return out + "\n";
}
