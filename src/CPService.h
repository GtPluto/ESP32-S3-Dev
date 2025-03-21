#pragma once
#include "BLEConfig.h"
#include <functional>

class CPService
{
public:
    CPService(NimBLEServer *server);
    void updateMeasurement(int16_t power);

private:
    NimBLEService *service;
    NimBLECharacteristic *cpMeasurementChar;
    NimBLECharacteristic *cpFeatureChar;
    NimBLECharacteristic *sensorLocationChar;

    // 特征值回调函数
    static void onCPMeasurementWrite(NimBLECharacteristic *pChar);
    static void onCPFeatureWrite(NimBLECharacteristic *pChar);
    static void onSensorLocationWrite(NimBLECharacteristic *pChar);
};