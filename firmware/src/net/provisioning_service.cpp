#include "net/provisioning_service.h"

#include <Arduino.h>
#include <DNSServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

#include "net/device_config.h"

namespace {

static DNSServer dnsServer;
static WebServer portalServer(80);

static const char *kPortalHtml = R"HTML(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>NomaBot Setup</title></head><body style="font-family:sans-serif;max-width:420px;margin:2em auto">
<h2>NomaBot Setup</h2>
<form method="POST" action="/save">
<label>WiFi SSID<br><input name="ssid" required style="width:100%"></label><br><br>
<label>WiFi Password<br><input name="pass" type="password" style="width:100%"></label><br><br>
<label>OpenWeatherMap API Key<br><input name="apikey" style="width:100%"></label><br><br>
<label>City (City,CC)<br><input name="city" value="Mumbai,IN" style="width:100%"></label><br><br>
<label>Timezone<br><input name="tz" value="IST-5:30" style="width:100%"></label><br><br>
<button type="submit">Save &amp; Reboot</button></form>
<p><a href="/status">Status</a></p>
<p style="margin-top:2em;font-size:0.85em;color:#666">Powered by: NomadProgrammer</p>
</body></html>
)HTML";

static void trimInPlace(char *value, size_t maxLen) {
  if (!value || maxLen == 0) {
    return;
  }
  size_t len = strnlen(value, maxLen);
  while (len > 0 && (value[len - 1] == ' ' || value[len - 1] == '\t' || value[len - 1] == '\r' ||
                     value[len - 1] == '\n')) {
    value[--len] = '\0';
  }
  size_t start = 0;
  while (value[start] == ' ' || value[start] == '\t') {
    ++start;
  }
  if (start > 0) {
    memmove(value, value + start, len - start + 1);
  }
}

static void copyPortalArg(char *dest, size_t destSize, const char *argName) {
  if (!portalServer.hasArg(argName)) {
    return;
  }
  strncpy(dest, portalServer.arg(argName).c_str(), destSize - 1);
  dest[destSize - 1] = '\0';
  trimInPlace(dest, destSize);
}

static void sendPortalError(const char *message) {
  String html = "<html><body style=\"font-family:sans-serif;max-width:420px;margin:2em auto\">";
  html += "<h3>Setup failed</h3><p>";
  html += message;
  html += "</p><p>Stay connected to WiFi <b>NomaBot</b> and <a href=\"/\">try again</a>.</p>";
  html += "</body></html>";
  portalServer.send(400, "text/html", html);
}

static void handleRoot() {
  portalServer.send(200, "text/html", kPortalHtml);
}

static void handleSave() {
  if (!portalServer.hasArg("ssid")) {
    Serial.println("Portal save: missing ssid arg");
    sendPortalError("SSID not received. Wait for the page to finish loading, then submit again.");
    return;
  }

  DeviceConfig cfg;
  deviceConfigLoad(cfg);
  copyPortalArg(cfg.wifiSsid, sizeof(cfg.wifiSsid), "ssid");
  copyPortalArg(cfg.wifiPass, sizeof(cfg.wifiPass), "pass");
  copyPortalArg(cfg.weatherApiKey, sizeof(cfg.weatherApiKey), "apikey");
  copyPortalArg(cfg.weatherCity, sizeof(cfg.weatherCity), "city");
  copyPortalArg(cfg.timezone, sizeof(cfg.timezone), "tz");

  if (cfg.wifiSsid[0] == '\0' || strcmp(cfg.wifiSsid, "your_wifi_ssid") == 0) {
    Serial.println("Portal save: invalid ssid");
    sendPortalError("Enter a valid WiFi network name (SSID).");
    return;
  }

  Serial.printf("Portal save: ssid=%s pass_len=%u api_len=%u\n", cfg.wifiSsid,
                static_cast<unsigned>(strlen(cfg.wifiPass)),
                static_cast<unsigned>(strlen(cfg.weatherApiKey)));

  if (!deviceConfigSaveWifi(cfg)) {
    portalServer.send(500, "text/plain", "Save failed");
    return;
  }

  if (!deviceConfigVerifySavedWifi(cfg)) {
    sendPortalError("Settings could not be verified after save. Please try again.");
    return;
  }

  String html = "<html><body style=\"font-family:sans-serif;max-width:420px;margin:2em auto\">";
  html += "<h3>Saved</h3><p>WiFi: <b>";
  html += cfg.wifiSsid;
  html += "</b></p><p>Rebooting...</p></body></html>";
  portalServer.send(200, "text/html", html);
  delay(500);
  LittleFS.end();
  ESP.restart();
}

static void handleStatus() {
  portalServer.send(200, "application/json", "{\"mode\":\"setup\"}");
}

static void handleCaptive() {
  portalServer.sendHeader("Location", "http://192.168.4.1/", true);
  portalServer.send(302, "text/plain", "");
}

}  // namespace

bool ProvisioningService::needsSetup() const {
  return !deviceConfigHasValidWifi();
}

bool ProvisioningService::startPortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("NomaBot", "eyes1234");
  dnsServer.start(53, "*", WiFi.softAPIP());
  portalServer.on("/", HTTP_GET, handleRoot);
  portalServer.on("/save", HTTP_POST, handleSave);
  portalServer.on("/status", HTTP_GET, handleStatus);
  portalServer.onNotFound(handleCaptive);
  portalServer.begin();
  _active = true;
  Serial.printf("NomaBot setup portal: http://%s\n", WiFi.softAPIP().toString().c_str());
  return true;
}

void ProvisioningService::tick() {
  if (!_active) {
    return;
  }
  dnsServer.processNextRequest();
  portalServer.handleClient();
}
