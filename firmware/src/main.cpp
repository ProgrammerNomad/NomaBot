#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include "ambient/display_mode.h"
#include "assets/pack_loader.h"
#include "character/character_runtime.h"
#include "debug/command_history.h"
#include "net/clock_service.h"
#include "net/weather_service.h"
#include "net/wifi_service.h"
#include "protocol/handler.h"
#include "renderer/lilygo_renderer.h"

static LilygoRenderer renderer;
static PackLoader packLoader;
static CharacterRuntime characterRuntime;
static ProtocolHandler protocol;
static String serialBuffer;
static bool bootOk = false;
static const char *bootFsStatus = "FAIL";
static const char *bootPackStatus = "FAIL";

WifiService gWifiService;
static ClockService gClockService;
static WeatherService gWeatherService;
static DisplayModeController gDisplayMode;

static void emitLine(const std::string &line) { Serial.print(line.c_str()); }

static void showBootError(const char *label) {
  renderer.fillScreen(0xF800);
  renderer.drawText(4, 20, label, 0xFFFF);
  renderer.drawText(4, 36, "Check pack / WiFi", 0xFFFF);
  renderer.drawText(4, 52, "Press RST to retry", 0xFFFF);
}

static void printBootBanner() {
  Serial.printf("Eyes Ambient FW %s | FS: %s | Pack: %s | render_mode=%s\n",
                NOMA_FIRMWARE_VERSION, bootFsStatus, bootPackStatus,
                bootOk ? renderModeName(characterRuntime.renderMode()) : "ERROR");
}

static bool bootCharacter() {
  if (!characterRuntime.loadCharacter(packLoader, "eyes")) {
    const char *label = packLoadErrorLabel(packLoader.lastError());
    Serial.printf("Pack load failed: eyes (%s)\n", label);
    bootPackStatus = "FAIL";
    return false;
  }
  bootPackStatus = "OK";
  return true;
}

static ProtocolResponse handleHello(const std::string &id, JsonObject params) {
  (void)params;
  JsonDocument data;
  data["protocol"] = 1;
  data["firmware"] = NOMA_FIRMWARE_VERSION;
  data["board"] = "LILYGO_T_DISPLAY_S3";
  data["character_id"] = "eyes";
  data["render_mode"] = bootOk ? renderModeName(characterRuntime.renderMode()) : "error";
  JsonObject display = data["display"].to<JsonObject>();
  display["width"] = renderer.width();
  display["height"] = renderer.height();
  JsonArray caps = data["caps"].to<JsonArray>();
  caps.add("diagnostics");

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

static ProtocolResponse handleGetStatus(const std::string &id, JsonObject) {
  JsonDocument data;
  data["firmware_version"] = NOMA_FIRMWARE_VERSION;
  data["animation"] = characterRuntime.currentAnimation();
  data["behavior"] = characterRuntime.currentBehavior();
  data["fps"] = characterRuntime.fps();
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

static ProtocolResponse handleDiagnostics(const std::string &id, JsonObject) {
  unsigned long now = millis();
  JsonDocument data;
  data["fps"] = characterRuntime.fps();
#if defined(ESP32)
  data["heap_free"] = ESP.getFreeHeap();
  data["psram_free"] = ESP.getFreePsram();
#endif
  data["behavior"] = characterRuntime.currentBehavior();
  data["clip"] = characterRuntime.brain().clipForBehavior();
  data["animation"] = characterRuntime.currentAnimation();
  data["frame"] = characterRuntime.currentFrame();
  data["time_in_behavior_sec"] = characterRuntime.timeInBehaviorSec(now);
  data["next_behavior"] = characterRuntime.nextBehavior();
  data["render_count"] = characterRuntime.renderCount();
  JsonArray history = data["command_history"].to<JsonArray>();
  gCommandHistory.appendToJson(history);

  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "diagnostics";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static void registerProtocolHandlers() {
  protocol.registerCommand("hello", handleHello);
  protocol.registerCommand("ping", handlePing);
  protocol.registerCommand("get_status", handleGetStatus);
  protocol.registerCommand("diagnostics", handleDiagnostics);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  renderer.begin();
  characterRuntime.begin(&renderer);
  registerProtocolHandlers();

  if (packLoader.mountFilesystem()) {
    bootFsStatus = "OK";
    bootOk = bootCharacter();
    if (!bootOk) {
      delay(500);
      bootOk = bootCharacter();
    }
    if (bootOk) {
      characterRuntime.setActivity("idle");
      gWifiService.begin();
      gClockService.begin(&characterRuntime);
      gWeatherService.begin(&characterRuntime);
      gDisplayMode.begin();
      characterRuntime.setDisplayMode(gDisplayMode.current());
      characterRuntime.present();
    } else {
      showBootError("EYES PACK FAIL");
    }
  } else {
    Serial.println("LittleFS mount failed");
    showBootError("FS MOUNT FAIL");
  }

  printBootBanner();
}

static void usbPoll() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      protocol.processLine(serialBuffer.c_str(), emitLine);
      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }
}

void loop() {
  unsigned long now = millis();
  usbPoll();
  if (bootOk) {
    gWifiService.tick();
    gClockService.tick();
    gWeatherService.tick();
    gDisplayMode.tick(now);
    if (gDisplayMode.consumeChanged()) {
      characterRuntime.setDisplayMode(gDisplayMode.current());
    }
    if (gDisplayMode.isEyesMode()) {
      characterRuntime.tick(now);
    }
    characterRuntime.present();
  }
  delay(1);
}
