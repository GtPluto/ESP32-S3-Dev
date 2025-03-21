#include "MockBikeDataSource.h"

MockBikeDataSource::MockBikeDataSource()
{
    // 初始化数据
    data.wheel_rev = 1;
    data.w_event_time = 0;
    data.crank_rev = 1;
    data.c_event_time = 0;
    data.power = MIN_POWER;
    data.speed = 0;
    data.cadence = 0;

    // 初始化随机数种子
    randomSeed(micros());
}

void MockBikeDataSource::update()
{
    unsigned long current_time = millis();

    // 安全检查：防止millis溢出或无效
    if (current_time == 0)
    {
        return;
    }

    // 每5秒随机调整目标值，模拟真实骑行过程中的变化
    if (current_time - last_target_update >= 5000)
    {
        // 在当前值的基础上随机调整 ±10%
        target_speed = current_speed * (1.0f + (random(-100, 100) / 1000.0f));
        target_cadence = current_cadence * (1.0f + (random(-100, 100) / 1000.0f));

        // 确保目标值在合理范围内
        target_speed = constrainValue(target_speed, 10.0f, 20.0f);
        target_cadence = constrainValue(target_cadence, 60.0f, 80.0f);

        last_target_update = current_time;
    }

    // 更新速度 (20Hz)
    if (current_time - last_wheel_update >= 50 || last_wheel_update == 0)
    {
        updateSpeed();
        last_wheel_update = current_time;
    }

    // 更新踏频 (20Hz)
    if (current_time - last_crank_update >= 50 || last_crank_update == 0)
    {
        updateCadence();
        last_crank_update = current_time;
    }

    // 更新功率 (20Hz)
    if (current_time - last_power_update >= 50 || last_power_update == 0)
    {
        updatePower();
        last_power_update = current_time;
    }

    // 每秒输出一次详细日志
    static unsigned long last_log = 0;
    if (current_time - last_log >= 1000)
    {
        Serial.printf("[BIKE] 速度: %.1f km/h (目标: %.1f), 踏频: %.1f rpm (目标: %.1f), 功率: %d W\n",
                      data.speed, target_speed, data.cadence, target_cadence, data.power);
        Serial.printf("[BIKE] 车轮: %u 转, 曲柄: %u 转, 时间戳 W:%u C:%u\n",
                      data.wheel_rev, data.crank_rev, data.w_event_time, data.c_event_time);
        last_log = current_time;
    }
}

void MockBikeDataSource::updateSpeed()
{
    // 安全检查
    if (isnan(target_speed) || isinf(target_speed))
    {
        target_speed = 25.0; // 默认安全值
    }
    if (isnan(current_speed) || isinf(current_speed))
    {
        current_speed = 0.0; // 重置为安全值
    }

    // 模拟速度变化
    float speed_diff = target_speed - current_speed;
    if (abs(speed_diff) > 0.1)
    {
        current_speed += speed_diff * 0.1; // 平滑变化
    }

    // 限制速度范围
    current_speed = constrainValue(current_speed, 0.0f, MAX_SPEED);

    // 计算轮转数
    float speed_ms = current_speed * (1000.0f / 3600.0f); // 转换为 m/s
    float wheel_rev_per_second = speed_ms / WHEEL_CIRCUMFERENCE;
    float wheel_rev_per_50ms = wheel_rev_per_second * 0.05f;

    // 累积小数部分，确保平滑更新
    static float wheel_rev_accumulator = 0.0f;
    wheel_rev_accumulator += wheel_rev_per_50ms;

    // 当累积值达到1或更大时更新计数
    uint32_t wheel_rev_increment = (uint32_t)wheel_rev_accumulator;
    if (wheel_rev_increment > 0)
    {
        wheel_rev_accumulator -= wheel_rev_increment;
        data.wheel_rev = safeAdd(data.wheel_rev, wheel_rev_increment);
        data.w_event_time = (uint16_t)(millis() & 0xFFFF);
    }

    data.speed = current_speed;
}

void MockBikeDataSource::updateCadence()
{
    // 安全检查
    if (isnan(target_cadence) || isinf(target_cadence))
    {
        target_cadence = 85.0; // 默认安全值
    }
    if (isnan(current_cadence) || isinf(current_cadence))
    {
        current_cadence = 0.0; // 重置为安全值
    }

    // 模拟踏频变化
    float cadence_diff = target_cadence - current_cadence;
    if (abs(cadence_diff) > 0.1)
    {
        current_cadence += cadence_diff * 0.05; // 更平滑的变化
    }

    // 限制踏频范围
    current_cadence = constrainValue(current_cadence, 0.0f, MAX_CADENCE);

    // 获取当前时间
    unsigned long current_time = millis();

    // 计算踏频增量
    float crank_rev_per_second = current_cadence / 60.0f;
    float crank_rev_per_50ms = crank_rev_per_second * 0.05f;

    // 累积小数部分，确保平滑更新
    static float crank_rev_accumulator = 0.0f;
    crank_rev_accumulator += crank_rev_per_50ms;

    // 更频繁地更新踏频计数
    if (crank_rev_accumulator >= 0.05f) // 降低更新阈值
    {
        uint32_t crank_rev_increment = 1;
        crank_rev_accumulator -= 0.05f;

        // 更新计数和时间戳
        data.crank_rev = safeAdd(data.crank_rev, crank_rev_increment);
        data.c_event_time = (uint16_t)(current_time & 0xFFFF);

        // 添加调试日志
        Serial.printf("[CRANK] 更新踏频计数: %u, 时间戳: %u, 当前踏频: %.1f, 目标踏频: %.1f\n",
                      data.crank_rev, data.c_event_time, current_cadence, target_cadence);
    }

    data.cadence = current_cadence;
}

void MockBikeDataSource::updatePower()
{
    // 安全检查
    if (isnan(current_speed) || isinf(current_speed) ||
        isnan(current_cadence) || isinf(current_cadence))
    {
        data.power = (int16_t)MIN_POWER;
        return;
    }

    // 基于速度和踏频的功率计算
    float base_power = RIDER_WEIGHT * POWER_FACTOR * (current_speed / 25.0);   // 25km/h作为基准速度
    float cadence_factor = constrainValue(current_cadence / 85.0, 0.0f, 1.5f); // 85rpm作为基准踏频

    // 计算最终功率
    float power = base_power * cadence_factor;

    // 添加一些随机波动（±5%）
    float rand_factor = 1.0 + (random(-50, 50) / 1000.0f);
    power *= rand_factor;

    // 限制功率范围
    power = constrainValue(power, MIN_POWER, MAX_POWER);

    // 设置功率
    data.power = (int16_t)power;
}

float MockBikeDataSource::constrainValue(float value, float min, float max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

uint32_t MockBikeDataSource::safeAdd(uint32_t a, uint32_t b)
{
    uint32_t result = a + b;
    if (result < a) // 检查溢出
    {
        return 0xFFFFFFFF;
    }
    return result;
}

uint16_t MockBikeDataSource::safeAdd(uint16_t a, uint16_t b)
{
    uint16_t result = a + b;
    if (result < a) // 检查溢出
    {
        return 0xFFFF;
    }
    return result;
}