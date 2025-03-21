#include "CSCService.h"
#include <Arduino.h>

CSCService::CSCService(NimBLEServer *server)
{
    if (!server)
    {
        Serial.println("[ERROR] CSCService: 无效的服务器指针");
        return;
    }

    try
    {
        // 创建 CSC 服务
        service = server->createService(CSC_UUID);
        if (!service)
        {
            Serial.println("[ERROR] CSCService: 创建服务失败");
            return;
        }

        // 创建 CSC 测量特征值
        cscMeasurementChar = service->createCharacteristic(
            CSC_MEASUREMENT_UUID,
            NIMBLE_PROPERTY::READ |
                NIMBLE_PROPERTY::NOTIFY);

        if (!cscMeasurementChar)
        {
            Serial.println("[ERROR] CSCService: 创建测量特征值失败");
            return;
        }

        // 初始化特征值
        uint8_t initialValue[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        cscMeasurementChar->setValue(initialValue, sizeof(initialValue));

        // 创建 CSC 特征值
        cscFeatureChar = service->createCharacteristic(
            CSC_FEATURE_UUID,
            NIMBLE_PROPERTY::READ);

        if (!cscFeatureChar)
        {
            Serial.println("[ERROR] CSCService: 创建特征值失败");
            return;
        }

        uint16_t features = 0x03; // 支持轮转和踏频数据
        cscFeatureChar->setValue((uint8_t *)&features, sizeof(features));

        // 创建传感器位置特征值
        sensorLocationChar = service->createCharacteristic(
            SENSOR_LOCATION_UUID,
            NIMBLE_PROPERTY::READ);

        if (!sensorLocationChar)
        {
            Serial.println("[ERROR] CSCService: 创建位置特征值失败");
            return;
        }

        uint8_t location = LOC_REAR_WHEEL;
        sensorLocationChar->setValue(&location, sizeof(location));

        // 启动服务
        service->start();
        Serial.println("[BLE] CSC服务启动成功");
    }
    catch (std::exception &e)
    {
        Serial.printf("[ERROR] CSCService构造函数异常: %s\n", e.what());
    }
    catch (...)
    {
        Serial.println("[ERROR] CSCService构造函数未知异常");
    }
}

void CSCService::updateMeasurement(uint32_t wheelRev, uint16_t wEventTime,
                                   uint32_t crankRev, uint16_t cEventTime)
{
    static unsigned long lastNotifyTime = 0;
    const unsigned long MIN_NOTIFY_INTERVAL = 200; // 最小通知间隔（毫秒）

    if (!service || !cscMeasurementChar)
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
        uint8_t data[11] = {0}; // 固定大小的缓冲区

        // 设置标志位 (0x03 表示同时包含车轮和曲柄数据)
        data[0] = 0x03;

        // 添加车轮数据 (小端序)
        data[1] = wheelRev & 0xFF;
        data[2] = (wheelRev >> 8) & 0xFF;
        data[3] = (wheelRev >> 16) & 0xFF;
        data[4] = (wheelRev >> 24) & 0xFF;

        // 添加车轮事件时间 (小端序)
        data[5] = wEventTime & 0xFF;
        data[6] = (wEventTime >> 8) & 0xFF;

        // 添加曲柄数据 (小端序)
        data[7] = crankRev & 0xFF;
        data[8] = (crankRev >> 8) & 0xFF;
        data[9] = (crankRev >> 16) & 0xFF;
        data[10] = (crankRev >> 24) & 0xFF;

        // 更新特征值并通知
        cscMeasurementChar->setValue(data, sizeof(data));
        cscMeasurementChar->notify();
        lastNotifyTime = currentTime;

        if (Serial.available())
        {
            Serial.printf("[CSC] 数据更新: w=%u, wt=%u, c=%u, ct=%u\n",
                          wheelRev, wEventTime, crankRev, cEventTime);
        }
    }
    catch (...)
    {
        Serial.println("[CSC] 更新数据失败");
    }
}

void CSCService::onCSCMeasurementWrite(NimBLECharacteristic *pChar)
{
    // 处理写入请求
}

void CSCService::onCSCFeatureWrite(NimBLECharacteristic *pChar)
{
    // 处理特征值写入
}

void CSCService::onSensorLocationWrite(NimBLECharacteristic *pChar)
{
    // 处理传感器位置写入
}