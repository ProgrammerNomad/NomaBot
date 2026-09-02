#include "net/provisioning_service.h"

#include <Arduino.h>
#include <DNSServer.h>
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

static void handleRoot() {
  portalServer.send(200, "text/html", kPortalHtml);
}

static void handleSave() {
  DeviceConfig cfg;
  deviceConfigLoad(cfg);
  if (portalServer.hasArg("ssid")) {
    strncpy(cfg.wifiSsid, portalServer.arg("ssid").c_str(), sizeof(cfg.wifiSsid) - 1);
  }
  if (portalServer.hasArg("pass")) {
    strncpy(cfg.wifiPass, portalServer.arg("pass").c_str(), sizeof(cfg.wifiPass) - 1);
  }
  if (portalServer.hasArg("apikey")) {
    strncpy(cfg.weatherApiKey, portalServer.arg("apikey").c_str(), sizeof(cfg.weatherApiKey) - 1);
  }
  if (portalServer.hasArg("city")) {
    strncpy(cfg.weatherCity, portalServer.arg("city").c_str(), sizeof(cfg.weatherCity) - 1);
  }
  if (portalServer.hasArg("tz")) {
    strncpy(cfg.timezone, portalServer.arg("tz").c_str(), sizeof(cfg.timezone) - 1);
  }
  if (!deviceConfigSaveWifi(cfg)) {
    portalServer.send(500, "text/plain", "Save failed");
    return;
  }
  portalServer.send(200, "text/html", "<html><body><h3>Saved. Rebooting...</h3></body></html>");
  delay(500);
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
  if (deviceConfigHasWifiFile()) {
    return false;
  }
  const DeviceConfig &cfg = deviceConfig();
  return !cfg.wifiSsid[0] || strcmp(cfg.wifiSsid, "your_wifi_ssid") == 0;
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
