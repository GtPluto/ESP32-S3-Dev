#pragma once
#include "BLEConfig.h"
#include <functional>

class CPService
{
public:
    CPService(BLEServer *server);
    void updateMeasurement(int16_t power);

private:
    BLEService *service;
    BLECharacteristic *cpMeasurementChar;
    BLECharacteristic *cpFeatureChar;
    BLECharacteristic *sensorLocationChar;

    // 特征值回调函数
    static void onCPMeasurementWrite(BLECharacteristic *pChar);
    static void onCPFeatureWrite(BLECharacteristic *pChar);
    static void onSensorLocationWrite(BLECharacteristic *pChar);
};