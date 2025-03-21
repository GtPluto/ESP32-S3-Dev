#pragma once
#include "BikeDataSource.h"
#include <Arduino.h>

// 模拟骑行数据源的实现 - 基于事件的更新版本
class MockBikeDataSource : public BikeDataSource
{
public:
    MockBikeDataSource();
    void update() override;
    Data getData() const override { return data; }

private:
    Data data;

    // 固定参数
    const float WHEEL_CIRCUMFERENCE = 2.105; // 轮子周长 (m) - 700C x 23mm 标准尺寸
    const float BASE_SPEED = 15.0;           // 基准速度 (km/h)
    const float BASE_CADENCE = 70.0;         // 基准踏频 (rpm)
    const float BASE_POWER = 100.0;          // 基准功率 (W)
    const float VARIATION = 0.05f;           // 允许5%的变化范围

    // 时间戳
    unsigned long last_update = 0;           // 上次更新时间
    unsigned long last_variation_update = 0; // 上次变化更新时间
    unsigned long last_wheel_event = 0;      // 上次车轮事件时间
    unsigned long last_crank_event = 0;      // 上次曲柄事件时间

    // 位置跟踪
    float wheel_position = 0.0f; // 车轮位置 (0.0-1.0，表示一圈中的位置)
    float crank_position = 0.0f; // 曲柄位置 (0.0-1.0，表示一圈中的位置)

    // 当前变化系数 (-1.0 到 1.0 之间)
    float speed_variation = 0.0f;
    float cadence_variation = 0.0f;
    float power_variation = 0.0f;

    // 辅助函数
    void updateData();            // 更新变化系数
    void updatePositions();       // 更新位置
    void checkAndTriggerEvents(); // 检查并触发事件
    float getRandomVariation();
    uint32_t safeAdd(uint32_t a, uint32_t b);
    uint16_t safeAdd(uint16_t a, uint16_t b);
};