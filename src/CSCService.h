#pragma once
#include "BLEConfig.h"
#include <functional>

class CSCService
{
public:
    CSCService(BLEServer *server);
    void updateMeasurement(uint32_t wheelRev, uint16_t wEventTime,
                           uint32_t crankRev, uint16_t cEventTime);

private:
    BLEService *service;
    BLECharacteristic *cscMeasurementChar;
    BLECharacteristic *cscFeatureChar;
    BLECharacteristic *sensorLocationChar;

    // 特征值回调函数
    void onCSCMeasurementWrite(BLECharacteristic *pChar);
    void onCSCFeatureWrite(BLECharacteristic *pChar);
    void onSensorLocationWrite(BLECharacteristic *pChar);
};