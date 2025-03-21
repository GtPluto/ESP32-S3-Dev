#pragma once
#include "BLEConfig.h"
#include <functional>

class CSCService
{
public:
    CSCService(NimBLEServer *server);
    void updateMeasurement(uint32_t wheelRev, uint16_t wEventTime,
                           uint32_t crankRev, uint16_t cEventTime);

private:
    NimBLEService *service;
    NimBLECharacteristic *cscMeasurementChar;
    NimBLECharacteristic *cscFeatureChar;
    NimBLECharacteristic *sensorLocationChar;

    // 特征值回调函数
    void onCSCMeasurementWrite(NimBLECharacteristic *pChar);
    void onCSCFeatureWrite(NimBLECharacteristic *pChar);
    void onSensorLocationWrite(NimBLECharacteristic *pChar);
};