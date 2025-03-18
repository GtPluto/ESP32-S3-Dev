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
    };

    void update()
    {
        // 这里添加实际传感器数据采集逻辑
        static uint32_t counter = 0;
        data.wheel_rev = counter * 10;
        data.w_event_time = millis() % 65536;
        data.crank_rev = counter * 2;
        data.c_event_time = (millis() + 500) % 65536;
        data.power = 100 + (counter % 50);
        counter++;
    }

    Data getData() const { return data; }

private:
    Data data;
};