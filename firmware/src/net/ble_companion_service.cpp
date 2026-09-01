#include "net/ble_companion_service.h"

#include <Arduino.h>

void BleCompanionService::begin() { Serial.println("BLE companion: optional (not enabled)"); }

void BleCompanionService::tick() {}

void BleCompanionService::notifyAlert(const char *message) { (void)message; }
