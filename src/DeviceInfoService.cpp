#include "DeviceInfoService.h"
#include "BLEConfig.h"

DeviceInfoService::DeviceInfoService(BLEServer *server)
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
}

void DeviceInfoService::createReadOnlyCharacteristic(BLEUUID uuid, const char *value)
{
    BLECharacteristic *charac = service->createCharacteristic(
        uuid,
        BLECharacteristic::PROPERTY_READ);
    charac->setValue(value);
}