#include "CPService.h"
#include "BLE2902.h"
#include <BLEDevice.h>
#include <Arduino.h>
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
    if (!cpMeasurementChar)
    {
        Serial.println("[ERROR] CPService: 测量特征值未初始化");
        return;
    }

    if (!service || !service->getServer())
    {
        Serial.println("[ERROR] CPService: 服务未初始化或服务器无效");
        return;
    }

    try
    {
        // 分配缓冲区
        uint8_t data[4] = {0}; // flags (1) + power (2) + reserved (1)
        size_t dataLen = 0;

        // 设置标志位 (0x20 表示瞬时功率测量)
        data[dataLen++] = 0x20;

        // 添加功率数据
        memcpy(&data[dataLen], &power, sizeof(power));
        dataLen += sizeof(power);

        // 添加保留字节
        data[dataLen++] = 0;

        // 验证数据长度
        if (dataLen > sizeof(data))
        {
            Serial.println("[CP] 数据长度超出缓冲区大小");
            return;
        }

        // 更新特征值
        cpMeasurementChar->setValue(data, dataLen);
        cpMeasurementChar->notify();
    }
    catch (const std::exception &e)
    {
        Serial.printf("[CP] 更新数据时发生异常: %s\n", e.what());
    }
    catch (...)
    {
        Serial.println("[CP] 更新数据时发生未知异常");
    }
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