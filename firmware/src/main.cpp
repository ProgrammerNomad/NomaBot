#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>

#include "ambient/audio_reactive_service.h"
#include "ambient/display_mode.h"
#include "ambient/minigame_controller.h"
#include "ambient/motion_service.h"
#include "ambient/night_mode.h"
#include "ambient/pomodoro_controller.h"
#include "assets/pack_loader.h"
#include "character/character_runtime.h"
#include "debug/command_history.h"
#include "net/ble_companion_service.h"
#include "net/calendar_service.h"
#include "net/clock_service.h"
#include "net/device_config.h"
#include "net/ota_service.h"
#include "net/provisioning_service.h"
#include "net/service_status.h"
#include "net/weather_service.h"
#include "net/web_dashboard_service.h"
#include "net/wifi_service.h"
#include "protocol/handler.h"
#include "renderer/lilygo_renderer.h"

static LilygoRenderer renderer;
static PackLoader packLoader;
static CharacterRuntime characterRuntime;
static ProtocolHandler protocol;
static constexpr size_t kSerialBufMax = 512;
static char serialBuffer[kSerialBufMax];
static size_t serialLength = 0;
static bool bootOk = false;
static const char *bootFsStatus = "FAIL";
static const char *bootPackStatus = "FAIL";
static const char *kCharacterIds[] = {"eyes"};
static size_t kCharacterIndex = 0;
static bool otaStarted = false;
static bool dashboardStarted = false;

WifiService gWifiService;
ClockService gClockService;
WeatherService gWeatherService;
static DisplayModeController gDisplayMode;
static ProvisioningService gProvisioning;
static OtaService gOtaService;
static NightModeController gNightMode;
static PomodoroController gPomodoro;
static CalendarService gCalendar;
static MinigameController gMinigame;
static WebDashboardService gWebDashboard;
static BleCompanionService gBle;
static AudioReactiveService gAudio;
static MotionService gMotion;

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
  if (!characterRuntime.loadCharacter(packLoader, kCharacterIds[kCharacterIndex])) {
    const char *label = packLoadErrorLabel(packLoader.lastError());
    Serial.printf("Pack load failed: %s (%s)\n", kCharacterIds[kCharacterIndex], label);
    bootPackStatus = "FAIL";
    return false;
  }
  bootPackStatus = "OK";
  return true;
}

static void cycleCharacterPack() {
  kCharacterIndex = (kCharacterIndex + 1) % (sizeof(kCharacterIds) / sizeof(kCharacterIds[0]));
  if (bootCharacter()) {
    characterRuntime.setActivity("idle");
    characterRuntime.present();
  }
}

static ProtocolResponse handleHello(const std::string &id, JsonObject params) {
  (void)params;
  JsonDocument data;
  data["protocol"] = 1;
  data["firmware"] = NOMA_FIRMWARE_VERSION;
  data["board"] = "LILYGO_T_DISPLAY_S3";
  data["character_id"] = kCharacterIds[kCharacterIndex];
  data["render_mode"] = bootOk ? renderModeName(characterRuntime.renderMode()) : "error";
  JsonObject display = data["display"].to<JsonObject>();
  display["width"] = renderer.width();
  display["height"] = renderer.height();
  JsonArray caps = data["caps"].to<JsonArray>();
  caps.add("diagnostics");
  caps.add("notify");

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

static ProtocolResponse handleNotify(const std::string &id, JsonObject params) {
  unsigned long duration = 2000;
  if (!params.isNull() && params["duration_ms"].is<unsigned long>()) {
    duration = params["duration_ms"].as<unsigned long>();
  }
  if (bootOk) {
    characterRuntime.triggerNotify(duration);
    gBle.notifyAlert("notify");
  }
  JsonDocument doc;
  doc["v"] = 1;
  doc["id"] = id;
  doc["type"] = "response";
  doc["cmd"] = "notify";
  doc["ok"] = true;
  std::string out;
  serializeJson(doc, out);
  return {out + "\n", true};
}

static ProtocolResponse handleGetStatus(const std::string &id, JsonObject) {
  ServiceStatus status = collectServiceStatus();
  JsonDocument data;
  data["firmware_version"] = NOMA_FIRMWARE_VERSION;
  data["animation"] = characterRuntime.currentAnimation();
  data["behavior"] = characterRuntime.currentBehavior();
  data["fps"] = characterRuntime.fps();
  data["wifi_connected"] = status.wifiConnected;
  data["clock_valid"] = status.clockValid;
  data["weather_ok"] = status.weatherOk;
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
  data["uptime_sec"] = now / 1000UL;
  data["wifi_rssi"] = gWifiService.rssi();
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
  protocol.registerCommand("notify", handleNotify);
  protocol.registerCommand("get_status", handleGetStatus);
  protocol.registerCommand("diagnostics", handleDiagnostics);
}

static void usbPoll() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      serialBuffer[serialLength] = '\0';
      protocol.processLine(serialBuffer, emitLine);
      serialLength = 0;
    } else if (serialLength + 1 < kSerialBufMax) {
      serialBuffer[serialLength++] = c;
    }
  }
}

static uint8_t gTransitionAlpha = 0;

static void updateTransition() {
  if (gTransitionAlpha == 0) {
    return;
  }
  gTransitionAlpha = gTransitionAlpha > 32 ? gTransitionAlpha - 32 : 0;
  characterRuntime.setTransitionAlpha(gTransitionAlpha);
}

static void handlePomodoroInput(const DisplayModeInput &input, unsigned long now) {
  if (gDisplayMode.current() != AmbientDisplayMode::PomodoroScreen) {
    return;
  }
  if (input.longPress) {
    gPomodoro.reset();
  } else if (input.shortPress) {
    gPomodoro.toggleStartPause(now);
  }
  characterRuntime.setPomodoro(gPomodoro.remainingSec(), gPomodoro.totalSec());
  if (gPomodoro.state() == PomodoroState::Running) {
    characterRuntime.setActivity("focus");
  }
  if (gPomodoro.consumeCompleted()) {
    characterRuntime.triggerNotify(3000);
    characterRuntime.setActivity("idle");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  DeviceConfig cfg;
  deviceConfigLoad(cfg);
  renderer.begin();
  characterRuntime.begin(&renderer);
  registerProtocolHandlers();
  gNightMode.begin(&renderer);

  if (packLoader.mountFilesystem()) {
    bootFsStatus = "OK";
    if (gProvisioning.needsSetup()) {
      gProvisioning.startPortal();
      showBootError("WIFI SETUP");
      renderer.drawText(4, 68, "Join EyesSetup", 0xFFFF);
      printBootBanner();
      return;
    }
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
      gPomodoro.begin();
      gCalendar.begin();
      gMinigame.begin();
      gAudio.begin();
      gMotion.begin();
      gBle.begin();
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

void loop() {
  unsigned long now = millis();
  usbPoll();

  if (gProvisioning.active()) {
    gProvisioning.tick();
    delay(10);
    return;
  }

  if (bootOk) {
    DisplayModeInput input = gDisplayMode.tick(now);
    handlePomodoroInput(input, now);

    gWifiService.tick();
    if (gWifiService.connected()) {
      if (!otaStarted) {
        gOtaService.begin();
        otaStarted = true;
      }
      if (!dashboardStarted) {
        gWebDashboard.begin();
        dashboardStarted = true;
      }
    }
    gOtaService.tick();
    gWebDashboard.tick();
    gClockService.tick();
    gWeatherService.tick();
    gCalendar.tick();
    gMinigame.tick(now);
    gAudio.tick();
    gMotion.tick();
    gPomodoro.tick(now);
    gNightMode.tick(gClockService.currentHour());

    if (gDisplayMode.consumeChanged()) {
      gTransitionAlpha = 255;
      characterRuntime.setTransitionAlpha(gTransitionAlpha);
      characterRuntime.setDisplayMode(gDisplayMode.current());
    }

    characterRuntime.setCalendarText(gCalendar.overlayText());
    characterRuntime.setMinigameText(gMinigame.statusText());
    characterRuntime.setPomodoro(gPomodoro.remainingSec(), gPomodoro.totalSec());
#if defined(ESP32)
    characterRuntime.setStats(now / 1000UL, ESP.getFreeHeap(), gWifiService.rssi(),
                              NOMA_FIRMWARE_VERSION);
#endif

    if (gDisplayMode.isEyesMode()) {
      characterRuntime.tick(now);
    }
    updateTransition();
    characterRuntime.present();
  }
  delay(1);
}
