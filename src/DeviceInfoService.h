#pragma once
#include "BLEConfig.h"

class DeviceInfoService
{
public:
    DeviceInfoService(BLEServer *server);

private:
    BLEService *service;
    void createReadOnlyCharacteristic(BLEUUID uuid, const char *value);
};