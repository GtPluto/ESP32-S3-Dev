#pragma once
#include "BLEConfig.h"

class DeviceInfoService
{
public:
    DeviceInfoService(NimBLEServer *server);

private:
    NimBLEService *service;
    void createReadOnlyCharacteristic(NimBLEUUID uuid, const char *value);
};