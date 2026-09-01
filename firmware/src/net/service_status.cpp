#include "net/service_status.h"

#include "net/clock_service.h"
#include "net/weather_service.h"
#include "net/wifi_service.h"

extern WifiService gWifiService;
extern ClockService gClockService;
extern WeatherService gWeatherService;

ServiceStatus collectServiceStatus() {
  ServiceStatus status;
  status.wifiConnected = gWifiService.connected();
  status.wifiReconnecting = gWifiService.reconnecting() && !status.wifiConnected;
  status.clockValid = gClockService.clockValid();
  status.weatherOk = gWeatherService.lastSuccess();
  status.weatherStale = gWeatherService.isStale();
  return status;
}
