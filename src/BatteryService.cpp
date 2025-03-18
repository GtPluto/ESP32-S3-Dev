#include "BatteryService.h"
#include "BLE2902.h"

BatteryService::BatteryService(BLEServer *server)
{
    service = server->createService(BAT_UUID);

    battLevelChar = service->createCharacteristic(
        BAT_LEVEL_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);

    // 设置初始值
    uint8_t level = 100;
    battLevelChar->setValue(&level, 1);
    battLevelChar->addDescriptor(new BLE2902());
}

void BatteryService::updateLevel(uint8_t level)
{
    battLevelChar->setValue(&level, 1);
    battLevelChar->notify();
}