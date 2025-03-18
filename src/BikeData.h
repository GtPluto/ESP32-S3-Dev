#pragma once
#include <stdint.h>
#include <Arduino.h>

class BikeData
{
public:
    struct Data
    {
        uint32_t wheel_rev;
        uint16_t w_event_time;
        uint16_t crank_rev;
        uint16_t c_event_time;
        int16_t power;
        float speed;   // 速度 (km/h)
        float cadence; // 踏频 (rpm)
    };

    BikeData();
    void update();
    Data getData() const { return data; }

private:
    Data data;

    // 模拟参数
    const float WHEEL_CIRCUMFERENCE = 2.0; // 轮子周长 (m)
    const float MAX_POWER = 300.0;         // 最大功率 (W)
    const float MIN_POWER = 50.0;          // 最小功率 (W)
    const float MAX_SPEED = 40.0;          // 最大速度 (km/h)
    const float MAX_CADENCE = 120.0;       // 最大踏频 (rpm)

    // 当前状态
    float current_speed = 0.0;   // 当前速度 (km/h)
    float current_cadence = 0.0; // 当前踏频 (rpm)
    float target_speed = 20.0;   // 目标速度 (km/h)
    float target_cadence = 90.0; // 目标踏频 (rpm)

    // 时间戳
    unsigned long last_wheel_update = 0;
    unsigned long last_crank_update = 0;
    unsigned long last_power_update = 0;

    // 更新函数
    void updateSpeed();
    void updateCadence();
    void updatePower();

    // 辅助函数
    float constrainValue(float value, float min, float max);
    uint32_t safeAdd(uint32_t a, uint32_t b);
    uint16_t safeAdd(uint16_t a, uint16_t b);
};