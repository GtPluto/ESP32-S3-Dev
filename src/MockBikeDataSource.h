#pragma once
#include "BikeDataSource.h"
#include <Arduino.h>

// 模拟骑行数据源的实现
class MockBikeDataSource : public BikeDataSource
{
public:
    MockBikeDataSource();
    void update() override;
    Data getData() const override { return data; }

private:
    Data data;

    // 模拟参数
    const float WHEEL_CIRCUMFERENCE = 2.105; // 轮子周长 (m) - 700C x 23mm 标准尺寸
    const float MAX_POWER = 800.0;           // 最大功率 (W)
    const float MIN_POWER = 0.0;             // 最小功率 (W)
    const float MAX_SPEED = 60.0;            // 最大速度 (km/h)
    const float MAX_CADENCE = 150.0;         // 最大踏频 (rpm)
    const float POWER_FACTOR = 2.8;          // 功率系数 (W/kg)
    const float RIDER_WEIGHT = 75.0;         // 骑行者体重 (kg)

    // 当前状态
    float current_speed = 0.0;   // 当前速度 (km/h)
    float current_cadence = 0.0; // 当前踏频 (rpm)
    float target_speed = 25.0;   // 目标速度 (km/h)
    float target_cadence = 85.0; // 目标踏频 (rpm)

    // 时间戳
    unsigned long last_wheel_update = 0;
    unsigned long last_crank_update = 0;
    unsigned long last_power_update = 0;
    unsigned long last_target_update = 0;

    // 更新函数
    void updateSpeed();
    void updateCadence();
    void updatePower();

    // 辅助函数
    float constrainValue(float value, float min, float max);
    uint32_t safeAdd(uint32_t a, uint32_t b);
    uint16_t safeAdd(uint16_t a, uint16_t b);
};