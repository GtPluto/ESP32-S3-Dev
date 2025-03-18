#include "BikeData.h"

BikeData::BikeData()
{
    // 初始化数据
    data.wheel_rev = 1; // 从1开始，确保有初始值
    data.w_event_time = 0;
    data.crank_rev = 1; // 从1开始，确保有初始值
    data.c_event_time = 0;
    data.power = MIN_POWER;
    data.speed = 0;
    data.cadence = 0;

    // 初始化当前状态
    current_speed = 15.0;   // 直接从目标速度开始
    current_cadence = 70.0; // 直接从目标踏频开始
    target_speed = 15.0;    // 初始目标速度
    target_cadence = 70.0;  // 初始目标踏频

    // 初始化时间戳
    last_wheel_update = 0;
    last_crank_update = 0;
    last_power_update = 0;

    // 初始化随机数种子
    randomSeed(micros());
}

float BikeData::constrainValue(float value, float min, float max)
{
    if (isnan(value) || isinf(value))
    {
        return min; // 如果值是NaN或Inf，返回最小值
    }
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

uint32_t BikeData::safeAdd(uint32_t a, uint32_t b)
{
    // 检查溢出
    if (b > UINT32_MAX - a)
    {
        return UINT32_MAX; // 溢出保护
    }
    return a + b;
}

uint16_t BikeData::safeAdd(uint16_t a, uint16_t b)
{
    // 检查溢出
    if (b > UINT16_MAX - a)
    {
        return UINT16_MAX; // 溢出保护
    }
    return a + b;
}

void BikeData::update()
{
    unsigned long current_time = millis();

    // 安全检查：防止millis溢出或无效
    if (current_time == 0)
    {
        return;
    }

    // 确保至少有一个数据更新
    bool dataUpdated = false;

    // 更新速度
    if (current_time - last_wheel_update >= 100 || last_wheel_update == 0)
    { // 10Hz 或初始更新
        updateSpeed();
        last_wheel_update = current_time;
        dataUpdated = true;
    }

    // 更新踏频
    if (current_time - last_crank_update >= 100 || last_crank_update == 0)
    { // 10Hz 或初始更新
        updateCadence();
        last_crank_update = current_time;
        dataUpdated = true;
    }

    // 更新功率
    if (current_time - last_power_update >= 100 || last_power_update == 0)
    { // 10Hz 或初始更新
        updatePower();
        last_power_update = current_time;
        dataUpdated = true;
    }

    // 如果没有数据更新，强制更新一次
    if (!dataUpdated)
    {
        updateSpeed();
        updateCadence();
        updatePower();
    }

    // 确保数据永远不为零
    if (data.wheel_rev == 0)
        data.wheel_rev = 1;
    if (data.crank_rev == 0)
        data.crank_rev = 1;
}

void BikeData::updateSpeed()
{
    // 安全检查
    if (isnan(target_speed) || isinf(target_speed))
    {
        target_speed = 15.0; // 默认安全值
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

    // 计算轮转数 - 使用安全计算
    float wheel_rev_per_second = constrainValue((current_speed * 1000.0) / (3600.0 * WHEEL_CIRCUMFERENCE), 0.0f, 20.0f);
    uint32_t wheel_rev_increment = (uint32_t)(wheel_rev_per_second * 0.1); // 0.1秒的轮转数

    // 限制增量为合理值，防止异常大的值
    wheel_rev_increment = min(wheel_rev_increment, (uint32_t)10);

    // 安全地增加轮转数
    data.wheel_rev = safeAdd(data.wheel_rev, wheel_rev_increment);
    data.w_event_time = (uint16_t)(millis() & 0xFFFF);
    data.speed = current_speed;
}

void BikeData::updateCadence()
{
    // 安全检查
    if (isnan(target_cadence) || isinf(target_cadence))
    {
        target_cadence = 70.0; // 默认安全值
    }
    if (isnan(current_cadence) || isinf(current_cadence))
    {
        current_cadence = 0.0; // 重置为安全值
    }

    // 模拟踏频变化
    float cadence_diff = target_cadence - current_cadence;
    if (abs(cadence_diff) > 0.1)
    {
        current_cadence += cadence_diff * 0.1; // 平滑变化
    }

    // 限制踏频范围
    current_cadence = constrainValue(current_cadence, 0.0f, MAX_CADENCE);

    // 计算踏频数 - 使用安全计算
    float crank_rev_per_second = constrainValue(current_cadence / 60.0, 0.0f, 5.0f);
    uint16_t crank_rev_increment = (uint16_t)(crank_rev_per_second * 0.1); // 0.1秒的踏频数

    // 限制增量为合理值，防止异常大的值
    crank_rev_increment = min(crank_rev_increment, (uint16_t)5);

    // 安全地增加踏频数
    data.crank_rev = safeAdd(data.crank_rev, crank_rev_increment);
    data.c_event_time = (uint16_t)(millis() & 0xFFFF);
    data.cadence = current_cadence;
}

void BikeData::updatePower()
{
    // 安全检查
    if (isnan(current_speed) || isinf(current_speed) ||
        isnan(current_cadence) || isinf(current_cadence) ||
        isnan(target_speed) || isinf(target_speed) ||
        isnan(target_cadence) || isinf(target_cadence))
    {
        // 如果有任何无效值，设置一个安全的功率值
        data.power = (int16_t)MIN_POWER;
        return;
    }

    // 基于速度和踏频计算功率，并添加安全约束
    float speed_factor = constrainValue(current_speed / max(target_speed, 0.1f), 0.0f, 1.0f);
    float cadence_factor = constrainValue(current_cadence / max(target_cadence, 0.1f), 0.0f, 1.0f);

    // 计算基础功率
    float base_power = MIN_POWER + (MAX_POWER - MIN_POWER) * speed_factor;

    // 根据踏频调整功率
    float power = base_power * (0.8 + 0.2 * cadence_factor);

    // 添加一些随机波动，但更为保守
    int16_t rand_factor = random(-50, 50);
    power *= (1.0 + (rand_factor / 1000.0));

    // 限制功率范围
    power = constrainValue(power, MIN_POWER, MAX_POWER);

    // 设置功率
    data.power = (int16_t)power;
}