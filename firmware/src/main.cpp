#include <Arduino.h>
#include <ArduinoJson.h>
#include "animation/engine.h"
#include "protocol/handler.h"
#include "renderer/lilygo_renderer.h"

static LilygoRenderer renderer;
static AnimationEngine animEngine;
static ProtocolHandler protocol;
static String serialBuffer;

static void emitLine(const std::string &line) { Serial.print(line.c_str()); }

static ProtocolResponse handleHello(const std::string &id, JsonObject params) {
  JsonDocument data;
  data["firmware_version"] = NOMA_FIRMWARE_VERSION;
  data["device_id"] = "lilygo-tdisplay-s3";
  JsonObject display = data["display"].to<JsonObject>();
  display["width"] = renderer.width();
  display["height"] = renderer.height();
  JsonArray caps = data["caps"].to<JsonArray>();
  caps.add("play_animation");
  caps.add("show_message");
  caps.add("set_background");
  caps.add("set_state");

  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "hello";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handlePing(const std::string &id, JsonObject) {
  JsonDocument data;
  data["pong"] = true;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "ping";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handlePlayAnimation(const std::string &id, JsonObject params) {
  const char *animation = params["animation"] | "idle";
  animEngine.setAnimation(animation);
  JsonDocument data;
  data["ok"] = true;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "play_animation";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handleShowMessage(const std::string &id, JsonObject params) {
  const char *text = params["text"] | "";
  animEngine.setMessage(text);
  JsonDocument data;
  data["ok"] = true;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "show_message";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handleGetStatus(const std::string &id, JsonObject) {
  JsonDocument data;
  data["firmware_version"] = NOMA_FIRMWARE_VERSION;
  data["active_animation"] = animEngine.currentAnimation().c_str();
  data["fps"] = 20;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "get_status";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static void drawFrame() {
  uint16_t bg = animEngine.blinkPhase() ? 0x1082 : 0x0000;
  if (animEngine.currentAnimation() == "coding") {
    bg = animEngine.blinkPhase() ? 0x04FF : 0x02A6;
  }
  renderer.fillScreen(bg);

  int cx = renderer.width() / 2;
  int cy = renderer.height() / 2;
  uint16_t body = animEngine.blinkPhase() ? 0x4C5F : 0x2D3F;
  renderer.fillRect(cx - 25, cy - 30, 50, 60, body);
  renderer.fillRect(cx - 15, cy - 55, 30, 30, body);

  renderer.drawText(4, 8, animEngine.currentAnimation().c_str(), 0xFFFF);

  if (!animEngine.message().empty()) {
    renderer.drawText(4, renderer.height() - 20, animEngine.message().c_str(), 0xFFFF);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  renderer.begin();
  animEngine.begin();

  protocol.registerCommand("hello", handleHello);
  protocol.registerCommand("ping", handlePing);
  protocol.registerCommand("play_animation", handlePlayAnimation);
  protocol.registerCommand("show_message", handleShowMessage);
  protocol.registerCommand("get_status", handleGetStatus);
  protocol.registerCommand("set_background", [](const std::string &id, JsonObject) {
    JsonDocument doc;
    doc["v"] = 1;
    doc["id"] = id;
    doc["type"] = "response";
    doc["cmd"] = "set_background";
    doc["ok"] = true;
    std::string out;
    serializeJson(doc, out);
    return ProtocolResponse{out + "\n", true};
  });
  protocol.registerCommand("set_state", [](const std::string &id, JsonObject) {
    JsonDocument doc;
    doc["v"] = 1;
    doc["id"] = id;
    doc["type"] = "response";
    doc["cmd"] = "set_state";
    doc["ok"] = true;
    std::string out;
    serializeJson(doc, out);
    return ProtocolResponse{out + "\n", true};
  });

  drawFrame();
}

void loop() {
  animEngine.tick(millis());

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      protocol.processLine(serialBuffer.c_str(), emitLine);
      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }

  static unsigned long lastDraw = 0;
  if (millis() - lastDraw > 100) {
    drawFrame();
    lastDraw = millis();
  }
}
