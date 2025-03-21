#pragma once
#include <stdint.h>

// 骑行数据源的抽象接口
class BikeDataSource
{
public:
    // 基础数据结构
    struct Data
    {
        uint32_t wheel_rev;    // 车轮转数
        uint16_t w_event_time; // 车轮事件时间
        uint32_t crank_rev;    // 曲柄转数
        uint16_t c_event_time; // 曲柄事件时间
        int16_t power;         // 功率 (W)
        float speed;           // 速度 (km/h)
        float cadence;         // 踏频 (rpm)
    };

    virtual ~BikeDataSource() = default;

    // 更新并获取最新的骑行数据
    virtual void update() = 0;

    // 获取当前数据
    virtual Data getData() const = 0;
};