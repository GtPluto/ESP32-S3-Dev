#pragma once
#include "BLEConfig.h"

class BatteryService
{
public:
    BatteryService(BLEServer *server);
    void updateLevel(uint8_t level);

private:
    BLEService *service;
    BLECharacteristic *battLevelChar;
};