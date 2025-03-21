#include "MockBikeDataSource.h"

MockBikeDataSource::MockBikeDataSource()
{
    // 初始化数据
    data.wheel_rev = 1;
    data.w_event_time = 0;
    data.crank_rev = 1;
    data.c_event_time = 0;
    data.power = BASE_POWER;
    data.speed = BASE_SPEED;
    data.cadence = BASE_CADENCE;

    // 初始化随机数种子
    randomSeed(micros());
}

float MockBikeDataSource::getRandomVariation()
{
    return (random(201) - 100) / 100.0f; // 返回 -1.0 到 1.0 之间的随机值
}

void MockBikeDataSource::updateData()
{
    // 更新变化系数
    speed_variation = getRandomVariation();
    cadence_variation = getRandomVariation();
    power_variation = getRandomVariation();

    // 应用变化到基准值
    data.speed = BASE_SPEED * (1.0f + speed_variation * VARIATION);
    data.cadence = BASE_CADENCE * (1.0f + cadence_variation * VARIATION);
    data.power = (int16_t)(BASE_POWER * (1.0f + power_variation * VARIATION));
}

void MockBikeDataSource::update()
{
    unsigned long current_time = millis();

    // 每2秒更新一次变化系数
    if (current_time - last_variation_update >= 2000)
    {
        updateData();
        last_variation_update = current_time;
    }

    // 更新位置并检查事件
    updatePositions();
    checkAndTriggerEvents();

    // 每秒输出一次详细日志
    static unsigned long last_log = 0;
    if (current_time - last_log >= 1000)
    {
        Serial.printf("[BIKE] 速度: %.1f km/h, 踏频: %.1f rpm, 功率: %d W\n",
                      data.speed, data.cadence, data.power);
        Serial.printf("[BIKE] 车轮位置: %.3f, 曲柄位置: %.3f\n",
                      wheel_position, crank_position);
        Serial.printf("[BIKE] 车轮: %u 转, 曲柄: %u 转\n",
                      data.wheel_rev, data.crank_rev);
        last_log = current_time;
    }

    last_update = current_time;
}

void MockBikeDataSource::updatePositions()
{
    unsigned long current_time = millis();
    float elapsed_seconds = (current_time - last_update) / 1000.0f;

    // 更新车轮位置
    float speed_ms = data.speed * (1000.0f / 3600.0f); // 转换为 m/s
    float wheel_revs_per_second = speed_ms / WHEEL_CIRCUMFERENCE;
    wheel_position += wheel_revs_per_second * elapsed_seconds;

    // 更新曲柄位置
    float crank_revs_per_second = data.cadence / 60.0f;
    crank_position += crank_revs_per_second * elapsed_seconds;
}

void MockBikeDataSource::checkAndTriggerEvents()
{
    unsigned long current_time = millis();

    // 检查并处理完整的车轮转动
    while (wheel_position >= 1.0f)
    {
        // 更新车轮计数和时间戳
        data.wheel_rev = safeAdd(data.wheel_rev, 1);
        data.w_event_time = (uint16_t)(current_time & 0xFFFF);
        last_wheel_event = current_time;

        wheel_position -= 1.0f;
    }

    // 检查并处理完整的曲柄转动
    while (crank_position >= 1.0f)
    {
        // 更新踏频计数和时间戳
        data.crank_rev = safeAdd(data.crank_rev, 1);
        data.c_event_time = (uint16_t)(current_time & 0xFFFF);
        last_crank_event = current_time;

        crank_position -= 1.0f;
    }
}

uint32_t MockBikeDataSource::safeAdd(uint32_t a, uint32_t b)
{
    uint32_t sum = a + b;
    return (sum < a) ? 0xFFFFFFFF : sum; // 处理溢出
}

uint16_t MockBikeDataSource::safeAdd(uint16_t a, uint16_t b)
{
    uint16_t sum = a + b;
    return (sum < a) ? 0xFFFF : sum; // 处理溢出
}