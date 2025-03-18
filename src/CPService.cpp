#include "CPService.h"
#include "BLE2902.h"
#include <BLEDevice.h>

CPService::CPService(BLEServer *server)
{
    // 创建 CP 服务
    service = server->createService(CP_UUID);

    // 创建 CP 测量特征值
    cpMeasurementChar = service->createCharacteristic(
        CP_MEASUREMENT_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    cpMeasurementChar->addDescriptor(new BLE2902());
    cpMeasurementChar->setCallbacks(new BLECharacteristicCallbacks());
    cpMeasurementChar->setValue((uint8_t *)0, 0);

    // 创建 CP 特征值
    cpFeatureChar = service->createCharacteristic(
        CP_FEATURE_UUID,
        BLECharacteristic::PROPERTY_READ);
    uint32_t features = 0x00; // 基本功率数据
    cpFeatureChar->setValue((uint8_t *)&features, sizeof(features));

    // 创建传感器位置特征值
    sensorLocationChar = service->createCharacteristic(
        SENSOR_LOCATION_UUID,
        BLECharacteristic::PROPERTY_READ);
    uint8_t location = LOC_REAR_WHEEL;
    sensorLocationChar->setValue((uint8_t *)&location, sizeof(location));

    // 启动服务
    service->start();
}

void CPService::updateMeasurement(int16_t power)
{
    uint8_t flags = 0x00; // 基本功率数据
    uint8_t data[3];

    data[0] = flags;
    data[1] = power & 0xFF;
    data[2] = (power >> 8) & 0xFF;

    cpMeasurementChar->setValue(data, sizeof(data));
    cpMeasurementChar->notify();
}

void CPService::onCPMeasurementWrite(BLECharacteristic *pChar)
{
    // 处理写入请求
}

void CPService::onCPFeatureWrite(BLECharacteristic *pChar)
{
    // 处理特征值写入
}

void CPService::onSensorLocationWrite(BLECharacteristic *pChar)
{
    // 处理传感器位置写入
}