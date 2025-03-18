#include "CSCService.h"
#include "BLE2902.h"
#include <Arduino.h>
#include <BLEDevice.h>

CSCService::CSCService(BLEServer *server)
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
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_NOTIFY);

        if (!cscMeasurementChar)
        {
            Serial.println("[ERROR] CSCService: 创建测量特征值失败");
            return;
        }

        auto descriptor = new BLE2902();
        if (!descriptor)
        {
            Serial.println("[ERROR] CSCService: 创建描述符失败");
            return;
        }
        cscMeasurementChar->addDescriptor(descriptor);

        // 初始化特征值
        uint8_t initialValue = 0;
        cscMeasurementChar->setValue(&initialValue, 1);

        // 创建 CSC 特征值
        cscFeatureChar = service->createCharacteristic(
            CSC_FEATURE_UUID,
            BLECharacteristic::PROPERTY_READ);

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
            BLECharacteristic::PROPERTY_READ);

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
    if (!service || !cscMeasurementChar)
    {
        Serial.println("[CSC] 服务或特征未初始化");
        return;
    }

    try
    {
        // 计算所需的总数据长度
        const size_t FLAGS_SIZE = 1;
        const size_t WHEEL_REV_SIZE = sizeof(uint32_t);
        const size_t WHEEL_EVENT_SIZE = sizeof(uint16_t);
        const size_t CRANK_REV_SIZE = sizeof(uint32_t);
        const size_t CRANK_EVENT_SIZE = sizeof(uint16_t);

        const size_t TOTAL_SIZE = FLAGS_SIZE + WHEEL_REV_SIZE + WHEEL_EVENT_SIZE +
                                  CRANK_REV_SIZE + CRANK_EVENT_SIZE;

        // 分配缓冲区
        uint8_t data[TOTAL_SIZE] = {0};
        size_t offset = 0;

        // 设置标志位 (0x03 表示同时包含车轮和曲柄数据)
        data[offset++] = 0x03;

        // 添加车轮数据 (小端序)
        data[offset++] = wheelRev & 0xFF;
        data[offset++] = (wheelRev >> 8) & 0xFF;
        data[offset++] = (wheelRev >> 16) & 0xFF;
        data[offset++] = (wheelRev >> 24) & 0xFF;

        // 添加车轮事件时间 (小端序)
        data[offset++] = wEventTime & 0xFF;
        data[offset++] = (wEventTime >> 8) & 0xFF;

        // 添加曲柄数据 (小端序)
        data[offset++] = crankRev & 0xFF;
        data[offset++] = (crankRev >> 8) & 0xFF;
        data[offset++] = (crankRev >> 16) & 0xFF;
        data[offset++] = (crankRev >> 24) & 0xFF;

        // 添加曲柄事件时间 (小端序)
        data[offset++] = cEventTime & 0xFF;
        data[offset++] = (cEventTime >> 8) & 0xFF;

        // 验证数据长度
        if (offset != TOTAL_SIZE)
        {
            Serial.printf("[CSC] 数据长度错误: 预期 %d, 实际 %d\n", TOTAL_SIZE, offset);
            return;
        }

        // 更新特征值
        cscMeasurementChar->setValue(data, TOTAL_SIZE);
        cscMeasurementChar->notify();

        if (Serial.available())
        {
            Serial.printf("[CSC] 数据更新成功: flags=0x%02X, wheel=%u, wTime=%u, crank=%u, cTime=%u\n",
                          data[0], wheelRev, wEventTime, crankRev, cEventTime);
        }
    }
    catch (const std::exception &e)
    {
        Serial.printf("[CSC] 更新数据时发生异常: %s\n", e.what());
    }
    catch (...)
    {
        Serial.println("[CSC] 更新数据时发生未知异常");
    }
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