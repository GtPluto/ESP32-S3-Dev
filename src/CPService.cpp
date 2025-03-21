#include "CPService.h"
#include <Arduino.h>

CPService::CPService(NimBLEServer *server)
{
    if (!server)
    {
        Serial.println("[ERROR] CPService: 无效的服务器指针");
        return;
    }

    try
    {
        // 创建 CP 服务
        service = server->createService(CP_UUID);
        if (!service)
        {
            Serial.println("[ERROR] CPService: 创建服务失败");
            return;
        }

        // 创建 CP 测量特征值
        cpMeasurementChar = service->createCharacteristic(
            CP_MEASUREMENT_UUID,
            NIMBLE_PROPERTY::READ |
                NIMBLE_PROPERTY::NOTIFY);

        if (!cpMeasurementChar)
        {
            Serial.println("[ERROR] CPService: 创建测量特征值失败");
            return;
        }

        // 初始化特征值
        uint8_t initialValue[] = {0x20, 0x00, 0x00, 0x00};
        cpMeasurementChar->setValue(initialValue, sizeof(initialValue));

        // 创建 CP 特征值
        cpFeatureChar = service->createCharacteristic(
            CP_FEATURE_UUID,
            NIMBLE_PROPERTY::READ);

        if (!cpFeatureChar)
        {
            Serial.println("[ERROR] CPService: 创建特征值失败");
            return;
        }

        uint32_t features = 0x00; // 基本功率数据
        cpFeatureChar->setValue((uint8_t *)&features, sizeof(features));

        // 创建传感器位置特征值
        sensorLocationChar = service->createCharacteristic(
            SENSOR_LOCATION_UUID,
            NIMBLE_PROPERTY::READ);

        if (!sensorLocationChar)
        {
            Serial.println("[ERROR] CPService: 创建位置特征值失败");
            return;
        }

        uint8_t location = LOC_REAR_WHEEL;
        sensorLocationChar->setValue(&location, sizeof(location));

        // 启动服务
        service->start();
        Serial.println("[BLE] CP服务启动成功");
    }
    catch (std::exception &e)
    {
        Serial.printf("[ERROR] CPService构造函数异常: %s\n", e.what());
    }
    catch (...)
    {
        Serial.println("[ERROR] CPService构造函数未知异常");
    }
}

void CPService::updateMeasurement(int16_t power)
{
    static unsigned long lastNotifyTime = 0;
    const unsigned long MIN_NOTIFY_INTERVAL = 200; // 最小通知间隔（毫秒）

    if (!service || !cpMeasurementChar)
    {
        return;
    }

    // 检查连接状态
    if (!NimBLEDevice::getServer()->getConnectedCount())
    {
        return;
    }

    // 检查通知间隔
    unsigned long currentTime = millis();
    if (currentTime - lastNotifyTime < MIN_NOTIFY_INTERVAL)
    {
        return;
    }

    try
    {
        // 准备数据
        uint8_t data[4] = {0}; // 固定大小的缓冲区

        // 设置标志位 (0x20 表示瞬时功率测量)
        data[0] = 0x20;

        // 添加功率数据 (小端序)
        data[1] = power & 0xFF;
        data[2] = (power >> 8) & 0xFF;

        // 保留字节
        data[3] = 0;

        // 更新特征值并通知
        cpMeasurementChar->setValue(data, sizeof(data));
        cpMeasurementChar->notify();
        lastNotifyTime = currentTime;

        if (Serial.available())
        {
            Serial.printf("[CP] 数据更新: power=%d\n", power);
        }
    }
    catch (...)
    {
        Serial.println("[CP] 更新数据失败");
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