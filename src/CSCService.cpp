#include "CSCService.h"
#include "BLE2902.h"
#include <BLEDevice.h>

CSCService::CSCService(BLEServer *server)
{
    // 创建 CSC 服务
    service = server->createService(CSC_UUID);

    // 创建 CSC 测量特征值
    cscMeasurementChar = service->createCharacteristic(
        CSC_MEASUREMENT_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    cscMeasurementChar->addDescriptor(new BLE2902());
    cscMeasurementChar->setCallbacks(new BLECharacteristicCallbacks());
    cscMeasurementChar->setValue((uint8_t *)0, 0);

    // 创建 CSC 特征值
    cscFeatureChar = service->createCharacteristic(
        CSC_FEATURE_UUID,
        BLECharacteristic::PROPERTY_READ);
    uint16_t features = 0x03; // 支持轮转和踏频数据
    cscFeatureChar->setValue((uint8_t *)&features, sizeof(features));

    // 创建传感器位置特征值
    sensorLocationChar = service->createCharacteristic(
        SENSOR_LOCATION_UUID,
        BLECharacteristic::PROPERTY_READ);
    uint8_t location = LOC_REAR_WHEEL;
    sensorLocationChar->setValue((uint8_t *)&location, sizeof(location));

    // 启动服务
    service->start();
}

void CSCService::updateMeasurement(uint32_t wheelRev, uint16_t wEventTime,
                                   uint16_t crankRev, uint16_t cEventTime)
{
    uint8_t flags = 0x03; // 同时包含轮转和踏频数据
    uint8_t data[7];

    data[0] = flags;
    data[1] = wheelRev & 0xFF;
    data[2] = (wheelRev >> 8) & 0xFF;
    data[3] = (wheelRev >> 16) & 0xFF;
    data[4] = (wheelRev >> 24) & 0xFF;
    data[5] = wEventTime & 0xFF;
    data[6] = (wEventTime >> 8) & 0xFF;

    cscMeasurementChar->setValue(data, sizeof(data));
    cscMeasurementChar->notify();
}

void CSCService::onCSCMeasurementWrite(BLECharacteristic *pChar)
{
    // 处理写入请求
}

void CSCService::onCSCFeatureWrite(BLECharacteristic *pChar)
{
    // 处理特征值写入
}

void CSCService::onSensorLocationWrite(BLECharacteristic *pChar)
{
    // 处理传感器位置写入
}