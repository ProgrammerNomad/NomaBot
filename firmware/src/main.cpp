#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include "assets/pack_loader.h"
#include "character/character_runtime.h"
#include "protocol/handler.h"
#include "renderer/lilygo_renderer.h"

static LilygoRenderer renderer;
static PackLoader packLoader;
static CharacterRuntime characterRuntime;
static ProtocolHandler protocol;
static String serialBuffer;
static String activeCharacterId = "nomabot";
static bool bootOk = false;
static const char *bootFsStatus = "FAIL";
static const char *bootPackStatus = "FAIL";

static void emitLine(const std::string &line) { Serial.print(line.c_str()); }

static void showBootError(const char *label) {
  renderer.fillScreen(0xF800);
  renderer.drawText(4, 20, label, 0xFFFF);
  renderer.drawText(4, 36, "Reset or load_character", 0xFFFF);
  renderer.drawText(4, 52, "via desktop USB", 0xFFFF);
}

static void printBootBanner() {
  Serial.printf(
      "NomaBot FW %s | FS: %s | Pack: %s | Character: %s\n", NOMA_FIRMWARE_VERSION,
      bootFsStatus, bootPackStatus, bootOk ? activeCharacterId.c_str() : "none");
}

static bool loadActiveCharacter() {
  File f = LittleFS.open("/active_character.json", "r");
  if (!f) {
    return false;
  }
  std::string text;
  while (f.available()) {
    text += static_cast<char>(f.read());
  }
  f.close();

  JsonDocument doc;
  if (deserializeJson(doc, text)) {
    return false;
  }
  const char *id = doc["character_id"] | "nomabot";
  activeCharacterId = id;
  return true;
}

static bool persistActiveCharacter(const char *characterId, const char *uuid) {
  JsonDocument doc;
  doc["character_id"] = characterId;
  if (uuid && uuid[0]) {
    doc["uuid"] = uuid;
  }
  File f = LittleFS.open("/active_character.json", "w");
  if (!f) {
    return false;
  }
  serializeJson(doc, f);
  f.close();
  return true;
}

static bool bootCharacter() {
  loadActiveCharacter();
  if (!characterRuntime.loadCharacter(packLoader, activeCharacterId.c_str())) {
    const char *label = packLoadErrorLabel(packLoader.lastError());
    Serial.printf("Failed to load character: %s (%s)\n", activeCharacterId.c_str(), label);
    bootPackStatus = "FAIL";
    showBootError(label);
    packLoader.listDirectory(std::string("/characters/") + activeCharacterId.c_str());
    return false;
  }
  bootPackStatus = "OK";
  bootOk = true;
  characterRuntime.render();
  return true;
}

static ProtocolResponse handleHello(const std::string &id, JsonObject params) {
  (void)params;
  JsonDocument data;
  data["protocol"] = 1;
  data["firmware"] = NOMA_FIRMWARE_VERSION;
  data["firmware_version"] = NOMA_FIRMWARE_VERSION;
  data["board"] = "LILYGO_T_DISPLAY_S3";
  data["device_id"] = "lilygo-tdisplay-s3";
#if defined(ESP32) && defined(ESP32S3)
  uint64_t mac = ESP.getEfuseMac();
  char serialBuf[24];
  snprintf(serialBuf, sizeof(serialBuf), "%02X:%02X:%02X:%02X:%02X:%02X",
           (uint8_t)(mac >> 40), (uint8_t)(mac >> 32), (uint8_t)(mac >> 24),
           (uint8_t)(mac >> 16), (uint8_t)(mac >> 8), (uint8_t)(mac));
  data["serial"] = serialBuf;
#else
  data["serial"] = "unknown";
#endif
  JsonObject display = data["display"].to<JsonObject>();
  display["width"] = renderer.width();
  display["height"] = renderer.height();
  display["fps"] = characterRuntime.fps() > 0 ? characterRuntime.fps() : 20;
  JsonArray caps = data["caps"].to<JsonArray>();
  caps.add("play_animation");
  caps.add("show_message");
  caps.add("set_background");
  caps.add("set_state");
  caps.add("load_character");
  caps.add("diagnostics");

  const PackInfo *info = characterRuntime.packInfo();
  if (info) {
    data["character_id"] = characterRuntime.characterId();
    data["pack_uuid"] = info->uuid;
  }

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
  characterRuntime.playAnimation(animation);
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
  characterRuntime.setMessage(text);
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

static ProtocolResponse handleSetState(const std::string &id, JsonObject params) {
  const char *state = params["state"] | "idle";
  characterRuntime.setState(state);
  JsonDocument data;
  data["ok"] = true;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "set_state";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handleSetBackground(const std::string &id, JsonObject params) {
  const char *bg = params["background"] | "office";
  characterRuntime.setBackground(bg);
  JsonDocument data;
  data["ok"] = true;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "set_background";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handleGetStatus(const std::string &id, JsonObject) {
  JsonDocument data;
  data["firmware_version"] = NOMA_FIRMWARE_VERSION;
  data["active_animation"] = characterRuntime.currentAnimation();
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

static ProtocolResponse handleLoadCharacter(const std::string &id, JsonObject params) {
  const char *characterId = params["character_id"] | "nomabot";
  characterRuntime.unload();
  packLoader.unload();

  JsonDocument data;
  bool ok = characterRuntime.loadCharacter(packLoader, characterId);
  if (ok) {
    activeCharacterId = characterId;
    bootOk = true;
    bootPackStatus = "OK";
    const PackInfo *info = characterRuntime.packInfo();
    persistActiveCharacter(characterId, info ? info->uuid.c_str() : nullptr);
    data["pack_id"] = info ? info->packId : characterId;
    data["uuid"] = info ? info->uuid : "";
    if (info) {
      JsonObject version = data["version"].to<JsonObject>();
      version["major"] = info->version.major;
      version["minor"] = info->version.minor;
      version["patch"] = info->version.patch;
      JsonObject display = data["display"].to<JsonObject>();
      display["profile"] = info->profile;
      display["width"] = info->displayWidth;
      display["height"] = info->displayHeight;
    }
    characterRuntime.render();
  } else {
    bootOk = false;
    bootPackStatus = "FAIL";
    data["error"] = packLoadErrorLabel(packLoader.lastError());
    showBootError(packLoadErrorLabel(packLoader.lastError()));
    packLoader.listDirectory(std::string("/characters/") + characterId);
  }

  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "load_character";
  doc["ok"] = ok;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", ok};
}

static ProtocolResponse handleDiagnostics(const std::string &id, JsonObject) {
  JsonDocument data;
  data["fps"] = characterRuntime.fps();
#if defined(ESP32)
  data["heap_free"] = ESP.getFreeHeap();
  data["psram_free"] = ESP.getFreePsram();
#else
  data["heap_free"] = 0;
  data["psram_free"] = 0;
#endif
  data["character_id"] = characterRuntime.characterId();
  const PackInfo *info = characterRuntime.packInfo();
  data["uuid"] = info ? info->uuid : "";
  data["animation"] = characterRuntime.currentAnimation();
  data["frame"] = characterRuntime.currentFrame();
  data["state"] = characterRuntime.currentState();

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
  protocol.registerCommand("play_animation", handlePlayAnimation);
  protocol.registerCommand("show_message", handleShowMessage);
  protocol.registerCommand("get_status", handleGetStatus);
  protocol.registerCommand("set_background", handleSetBackground);
  protocol.registerCommand("set_state", handleSetState);
  protocol.registerCommand("load_character", handleLoadCharacter);
  protocol.registerCommand("diagnostics", handleDiagnostics);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  renderer.begin();
  characterRuntime.begin(&renderer);
  registerProtocolHandlers();

  if (!packLoader.mountFilesystem()) {
    Serial.println("LittleFS mount failed");
    showBootError("FS FAIL");
    printBootBanner();
    return;
  }
  bootFsStatus = "OK";

  if (!bootCharacter()) {
    printBootBanner();
    return;
  }

  printBootBanner();
}

void loop() {
  unsigned long now = millis();
  if (bootOk) {
    characterRuntime.tick(now);
  }

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      protocol.processLine(serialBuffer.c_str(), emitLine);
      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }

  if (!bootOk) {
    return;
  }

  static unsigned long lastDraw = 0;
  if (now - lastDraw > 50) {
    characterRuntime.render();
    lastDraw = now;
  }
}
