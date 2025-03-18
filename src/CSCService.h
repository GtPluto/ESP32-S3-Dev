#pragma once
#include "BLEConfig.h"
#include <functional>

class CSCService
{
public:
    CSCService(BLEServer *server);
    void updateMeasurement(uint32_t wheelRev, uint16_t wEventTime,
                           uint16_t crankRev, uint16_t cEventTime);

private:
    BLEService *service;
    BLECharacteristic *cscMeasurementChar;
    BLECharacteristic *cscFeatureChar;
    BLECharacteristic *sensorLocationChar;

    // 特征值回调函数
    static void onCSCMeasurementWrite(BLECharacteristic *pChar);
    static void onCSCFeatureWrite(BLECharacteristic *pChar);
    static void onSensorLocationWrite(BLECharacteristic *pChar);
};