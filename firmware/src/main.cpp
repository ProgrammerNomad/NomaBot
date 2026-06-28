#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include "assets/pack_loader.h"
#include "character/character_runtime.h"
#include "debug/command_history.h"
#include "protocol/handler.h"
#include "renderer/lilygo_renderer.h"

static LilygoRenderer renderer;
static PackLoader packLoader;
static CharacterRuntime characterRuntime;
static ProtocolHandler protocol;
static String serialBuffer;
static String activeCharacterId = "nomabot";
static bool bootOk = false;
static bool textModeBoot = false;
static const char *bootFsStatus = "FAIL";
static const char *bootPackStatus = "FAIL";

static void emitLine(const std::string &line) { Serial.print(line.c_str()); }

#define RECORD_CMD(name, detail) gCommandHistory.record(name, detail, millis())

static void showBootError(const char *label) {
  renderer.fillScreen(0xF800);
  renderer.drawText(4, 20, label, 0xFFFF);
  renderer.drawText(4, 36, "Reset or load_character", 0xFFFF);
  renderer.drawText(4, 52, "via desktop USB", 0xFFFF);
}

static void printBootBanner() {
  if (textModeBoot) {
    Serial.printf(
        "NomaBot FW %s | Mode: TEXT | Life: %s | Activity: %s | Emotion: %s | Behavior: %s\n",
        NOMA_FIRMWARE_VERSION, characterRuntime.lifeMode(), characterRuntime.currentActivity(),
        characterRuntime.currentEmotion(), characterRuntime.currentBehavior());
    return;
  }
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
    Serial.printf("Pack load failed: %s (%s) — falling back to text mode\n",
                  activeCharacterId.c_str(), label);
    bootPackStatus = "FAIL";
    return false;
  }
  bootPackStatus = "OK";
  return true;
}

static void startTextModeBoot() {
  textModeBoot = true;
  bootOk = true;
  characterRuntime.useBehaviorDefaults();
  characterRuntime.setLifeMode("work");
  characterRuntime.setActivity("idle");
  characterRuntime.present();
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
  caps.add("set_activity");
  caps.add("set_emotion");
  caps.add("set_life_mode");
  caps.add("trigger_habit");
  caps.add("set_season");
  caps.add("load_character");
  caps.add("diagnostics");

  const PackInfo *info = characterRuntime.packInfo();
  if (info) {
    data["character_id"] = characterRuntime.characterId();
    data["pack_uuid"] = info->uuid;
  } else {
    data["character_id"] = "nomabot";
  }
  data["render_mode"] = textModeBoot ? "text" : renderModeName(characterRuntime.renderMode());

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
  RECORD_CMD("play_animation", animation);
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
  const char *overlayId = params["id"] | "anonymous";
  const char *text = params["text"] | "";
  int priority = params["priority"] | 2;
  unsigned long durationMs = params["duration_ms"] | 5000UL;
  characterRuntime.setMessage(overlayId, text, priority, durationMs);
  RECORD_CMD("show_message", overlayId);
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
  characterRuntime.applyActivityCommand(state);
  RECORD_CMD("set_state", state);
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

static ProtocolResponse handleSetActivity(const std::string &id, JsonObject params) {
  const char *activity = params["activity"] | "idle";
  characterRuntime.applyActivityCommand(activity);
  RECORD_CMD("set_activity", activity);
  JsonDocument data;
  data["ok"] = true;
  data["activity"] = activity;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "set_activity";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handleSetEmotion(const std::string &id, JsonObject params) {
  const char *emotion = params["emotion"] | "neutral";
  characterRuntime.setEmotion(emotion);
  RECORD_CMD("set_emotion", emotion);
  JsonDocument data;
  data["ok"] = true;
  data["emotion"] = emotion;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "set_emotion";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handleSetLifeMode(const std::string &id, JsonObject params) {
  const char *mode = params["mode"] | "work";
  characterRuntime.setLifeMode(mode);
  RECORD_CMD("set_life_mode", mode);
  JsonDocument data;
  data["ok"] = true;
  data["mode"] = mode;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "set_life_mode";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handleTriggerHabit(const std::string &id, JsonObject params) {
  const char *habit = params["habit"] | "";
  characterRuntime.triggerHabit(habit);
  RECORD_CMD("trigger_habit", habit);
  JsonDocument data;
  data["ok"] = true;
  data["habit"] = habit;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "trigger_habit";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handleSetSeason(const std::string &id, JsonObject params) {
  const char *season = params["season"] | "spring";
  characterRuntime.setSeason(season);
  RECORD_CMD("set_season", season);
  JsonDocument data;
  data["ok"] = true;
  data["season"] = season;
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "set_season";
  doc["ok"] = true;
  doc["data"] = data;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handleSetBackground(const std::string &id, JsonObject params) {
  const char *bg = params["background"] | "office";
  characterRuntime.setBackground(bg);
  RECORD_CMD("set_background", bg);
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
  data["activity"] = characterRuntime.currentActivity();
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

static ProtocolResponse handleLoadCharacter(const std::string &id, JsonObject params) {
  const char *characterId = params["character_id"] | "nomabot";
  characterRuntime.unload();
  packLoader.unload();

  JsonDocument data;
  bool ok = false;
  if (packLoader.mountFilesystem() && characterRuntime.loadCharacter(packLoader, characterId)) {
    ok = true;
    activeCharacterId = characterId;
    bootOk = true;
    textModeBoot = false;
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
    characterRuntime.present();
  } else {
    bootOk = true;
    textModeBoot = true;
    bootPackStatus = "FAIL";
    characterRuntime.useBehaviorDefaults();
    data["error"] = packLoadErrorLabel(packLoader.lastError());
    data["render_mode"] = "text";
    characterRuntime.present();
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
  return {out + "\n", true};
}

static void appendDirtyLast(JsonArray arr, DirtyFlags flags) {
  if (hasDirty(flags, DirtyHeader)) {
    arr.add("Header");
  }
  if (hasDirty(flags, DirtyMeta)) {
    arr.add("Meta");
  }
  if (hasDirty(flags, DirtyEnergy)) {
    arr.add("Energy");
  }
  if (hasDirty(flags, DirtyBehavior)) {
    arr.add("Behavior");
  }
  if (hasDirty(flags, DirtyMessage)) {
    arr.add("Message");
  }
  if (hasDirty(flags, DirtyCharacter)) {
    arr.add("Character");
  }
  if (hasDirty(flags, DirtyBackground)) {
    arr.add("Background");
  }
  if (flags == DirtyFull) {
    arr.add("Full");
  }
}

static ProtocolResponse handleDiagnostics(const std::string &id, JsonObject) {
  unsigned long now = millis();
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
  data["life_mode"] = characterRuntime.lifeMode();
  data["activity"] = characterRuntime.currentActivity();
  data["emotion"] = characterRuntime.currentEmotion();
  data["behavior"] = characterRuntime.currentBehavior();
  data["behavior_label"] = characterRuntime.brain().behaviorLabel();
  data["clip"] = characterRuntime.brain().clipForBehavior();
  data["render_mode"] = textModeBoot ? "text" : renderModeName(characterRuntime.renderMode());
  data["time_in_behavior_sec"] = characterRuntime.timeInBehaviorSec(now);
  data["next_behavior"] = characterRuntime.nextBehavior();
  data["animation"] = characterRuntime.currentAnimation();
  data["frame"] = characterRuntime.currentFrame();
  data["state"] = characterRuntime.currentActivity();
  data["energy"] = characterRuntime.energy();
  data["boredom"] = characterRuntime.boredom();
  data["goal"] = characterRuntime.goal();
  data["goal_progress"] = characterRuntime.goalProgress();
  JsonObject shortMem = data["short_memory"].to<JsonObject>();
  int coffeeAgo = characterRuntime.lastCoffeeMinAgo(now);
  if (coffeeAgo >= 0) {
    shortMem["last_coffee_min_ago"] = coffeeAgo;
  }
  data["last_command_source"] = characterRuntime.lastCommandSource();
  data["render_count"] = characterRuntime.renderCount();
  data["last_render_ms"] = characterRuntime.lastRenderMs();
  data["brain_tick_ms"] = characterRuntime.lastBrainTickMs();
  data["render_ms"] = characterRuntime.lastRenderMs();
  data["queue_depth"] = characterRuntime.overlayQueueDepth();
  SceneDiagnostics sceneDiag = characterRuntime.lastSceneDiagnostics();
  data["scene"] = sceneDiag.scene ? sceneDiag.scene : "";
  data["body"] = sceneDiag.body ? sceneDiag.body : "";
  data["eyes"] = sceneDiag.eyes ? sceneDiag.eyes : "";
  data["overlay"] = sceneDiag.overlay ? sceneDiag.overlay : "";
  data["render_objects"] = sceneDiag.renderObjects;
  if (const char *bodySprite = characterRuntime.bodySpriteId()) {
    data["body_sprite_id"] = bodySprite;
  }
  data["clip_frame_index"] = characterRuntime.currentFrame();
  JsonArray dirtyLast = data["dirty_last"].to<JsonArray>();
  appendDirtyLast(dirtyLast, characterRuntime.lastDirtyFlags());
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
  protocol.registerCommand("play_animation", handlePlayAnimation);
  protocol.registerCommand("show_message", handleShowMessage);
  protocol.registerCommand("get_status", handleGetStatus);
  protocol.registerCommand("set_background", handleSetBackground);
  protocol.registerCommand("set_state", handleSetState);
  protocol.registerCommand("set_activity", handleSetActivity);
  protocol.registerCommand("set_emotion", handleSetEmotion);
  protocol.registerCommand("set_life_mode", handleSetLifeMode);
  protocol.registerCommand("trigger_habit", handleTriggerHabit);
  protocol.registerCommand("set_season", handleSetSeason);
  protocol.registerCommand("load_character", handleLoadCharacter);
  protocol.registerCommand("diagnostics", handleDiagnostics);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  renderer.begin();
  characterRuntime.begin(&renderer);
  registerProtocolHandlers();

  bool fsMounted = packLoader.mountFilesystem();
  if (fsMounted) {
    bootFsStatus = "OK";
    if (bootCharacter()) {
      bootOk = true;
      characterRuntime.setLifeMode("work");
      characterRuntime.setActivity("idle");
      characterRuntime.present();
    } else {
      startTextModeBoot();
    }
  } else {
    Serial.println("LittleFS mount failed — text mode boot");
    startTextModeBoot();
  }

  printBootBanner();
}

// v0.4.1 loop freeze — do not reorder without ADR.
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

static void brainTick(unsigned long nowMs) { characterRuntime.tick(nowMs); }

static void rendererPresent() { characterRuntime.present(); }

static void sleepYield() { delay(1); }

void loop() {
  unsigned long now = millis();
  usbPoll();
  if (bootOk) {
    brainTick(now);
    rendererPresent();
  }
  sleepYield();
}
