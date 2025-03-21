#include "BatteryService.h"
#include <Arduino.h>

BatteryService::BatteryService(NimBLEServer *server)
{
    service = server->createService(BAT_UUID);

    battLevelChar = service->createCharacteristic(
        BAT_LEVEL_UUID,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY);

    // 设置初始值
    uint8_t level = 100;
    battLevelChar->setValue(&level, 1);
    service->start();
}

void BatteryService::updateLevel(uint8_t level)
{
    battLevelChar->setValue(&level, 1);
    battLevelChar->notify();
}