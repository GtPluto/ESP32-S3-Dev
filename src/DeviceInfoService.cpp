#include "DeviceInfoService.h"
#include <Arduino.h>

DeviceInfoService::DeviceInfoService(NimBLEServer *server)
{
    service = server->createService(DI_UUID);

    // 系统ID (64-bit)
    uint64_t systemId = 0x0000022001100000;
    createReadOnlyCharacteristic(DI_SYSTEM_ID_UUID, (char *)&systemId);

    // 文本型特征
    createReadOnlyCharacteristic(DI_MODEL_NUMBER_UUID, "Keiser M to GATT");
    createReadOnlyCharacteristic(DI_SERIAL_NUMBER_UUID, "12345678");
    createReadOnlyCharacteristic(DI_FIRMWARE_REV_UUID, "0.0.1");
    createReadOnlyCharacteristic(DI_HARDWARE_REV_UUID, "0.1.1");
    createReadOnlyCharacteristic(DI_SOFTWARE_REV_UUID, "1.0beta");
    createReadOnlyCharacteristic(DI_MANUFACTURER_UUID, "t-j");

    service->start();
}

void DeviceInfoService::createReadOnlyCharacteristic(NimBLEUUID uuid, const char *value)
{
    NimBLECharacteristic *charac = service->createCharacteristic(
        uuid,
        NIMBLE_PROPERTY::READ);
    charac->setValue(value);
}