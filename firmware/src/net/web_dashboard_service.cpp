#include "net/web_dashboard_service.h"

#include <WebServer.h>
#include <WiFi.h>

#include "net/wifi_service.h"

extern WifiService gWifiService;

namespace {
WebServer dashboard(8080);
}

void WebDashboardService::begin() {
  if (!gWifiService.connected()) {
    return;
  }
  dashboard.on("/", HTTP_GET, []() {
    String body = "{\"status\":\"ok\",\"host\":\"eyes-ambient\"}";
    dashboard.send(200, "application/json", body);
  });
  dashboard.begin();
}

void WebDashboardService::tick() {
  if (gWifiService.connected()) {
    dashboard.handleClient();
  }
}
