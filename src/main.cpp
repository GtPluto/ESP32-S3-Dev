#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <NimBLEServer.h>
#include <NimBLEAdvertising.h>
#include <Ticker.h>

// ====== BLE 配置 ======
#define DEVICE_NAME "BikeSensor"
#define CSC_SERVICE_UUID "1816"
#define CSC_MEASUREMENT_UUID "2A5B"
#define CSC_FEATURE_UUID "2A5C"

// BLE对象声明
NimBLEServer *pServer;
NimBLEService *pCSCService;
NimBLECharacteristic *pMeasurementChar;
NimBLECharacteristic *pFeatureChar;

uint32_t cumulativeWheelRevs = 0;       // 累积轮转次数
uint16_t lastWheelEventTime = 0;        // 上次轮转时间（1/1024秒单位）
uint16_t cumulativeCrankRevs = 0;       // 累积曲柄转数
uint16_t lastCrankEventTime = 0;        // 上次曲柄时间（1/1024秒单位）
const float WHEEL_CIRCUMFERENCE = 2.07; // 车轮周长（米）典型公路车值
BLECharacteristic *pCscMeasurement;     // CSC测量特征指针（需在setup初始化）

// 广告管理
Ticker advTicker;
bool useLongInterval = false;

// ====== 隐私和安全配置 ======
void setupSecurity()
{
    // 启用绑定、MITM保护和LE安全连接
    NimBLEDevice::setSecurityAuth(true, true, true);
    // 设置无用户交互的I/O能力
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
}

// ====== 广告配置 ======
void startAdvertising(uint32_t intervalMs)
{
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->stop();

    // 构建广告数据包
    NimBLEAdvertisementData advData;
    advData.setFlags(BLE_HS_ADV_F_DISC_GEN);                   // 通用可发现模式
    advData.setCompleteServices(NimBLEUUID(CSC_SERVICE_UUID)); // 服务UUID
    advData.setName(DEVICE_NAME);                              // 设备名称
    advData.addTxPower();                                      // 发射功率

    // 构建扫描响应数据包
    NimBLEAdvertisementData scanResp;
    scanResp.setCompleteServices(NimBLEUUID(CSC_SERVICE_UUID));

    // 设置广告参数
    pAdvertising->setAdvertisementData(advData);
    pAdvertising->setScanResponseData(scanResp);

    // 精确设置广告间隔（0.625ms单位）
    uint32_t intervalUnits = static_cast<uint32_t>(intervalMs / 0.625);
    pAdvertising->setMinInterval(intervalUnits);
    pAdvertising->setMaxInterval(intervalUnits);

    // 启动广告
    pAdvertising->start();
}

// ====== CSC数据生成 ======
void sendCadenceData(uint32_t wheelRevs, uint16_t lastWheelTime,
                     uint16_t crankRevs, uint16_t lastCrankTime)
{
    uint8_t data[11];
    data[0] = 0x03; // 标志位：同时包含轮速和曲柄数据

    // 小端格式写入数据
    memcpy(&data[1], &wheelRevs, 4);
    memcpy(&data[5], &lastWheelTime, 2);
    memcpy(&data[7], &crankRevs, 2);
    memcpy(&data[9], &lastCrankTime, 2);

    pMeasurementChar->setValue(data, sizeof(data));
    pMeasurementChar->notify();
}

// ====== 主程序 ======
void setup()
{
    Serial.begin(115200);

    // 初始化BLE堆栈
    NimBLEDevice::init(DEVICE_NAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); // 设置最大发射功率

    // 安全配置
    setupSecurity();

    // 创建BLE服务器
    pServer = NimBLEDevice::createServer();

    // 创建CSC服务
    pCSCService = pServer->createService(CSC_SERVICE_UUID);

    // 配置测量特征（通知功能）
    pMeasurementChar = pCSCService->createCharacteristic(
        CSC_MEASUREMENT_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    // 配置特性特征
    uint16_t features = 0x0003; // 支持轮速和曲柄传感器
    pFeatureChar = pCSCService->createCharacteristic(
        CSC_FEATURE_UUID,
        NIMBLE_PROPERTY::READ);
    pFeatureChar->setValue(reinterpret_cast<uint8_t *>(&features), sizeof(features));

    // 启动服务
    pCSCService->start();

    // 初始广告（20ms间隔）
    startAdvertising(20);

    // 设置30秒后切换广告间隔
    advTicker.attach(30, []()
                     { useLongInterval = true; });

    pCscMeasurement = pCSCService->createCharacteristic(
        "2A5B",
        NIMBLE_PROPERTY::NOTIFY);
}

// 更新轮转数据（基于物理公式）
void updateWheelData(float speedKmh)
{
    const uint32_t currentTime = millis();

    // 计算时间差（转换为秒）
    float timeDiff = (currentTime - lastWheelEventTime) / 1000.0;

    // 计算轮转增量：速度(km/h) -> 米/秒 -> 转数/秒
    float wheelRevsDelta = (speedKmh * 1000 / 3600) / WHEEL_CIRCUMFERENCE;
    cumulativeWheelRevs += round(wheelRevsDelta * timeDiff);

    // 更新时间戳（转换为1/1024秒单位）
    lastWheelEventTime = (currentTime * 1024) / 1000;
}

// 更新曲柄数据（基于踏频）
void updateCrankData(int cadenceRpm)
{
    const uint32_t currentTime = millis();

    // 计算时间差（转换为分钟）
    float timeDiff = (currentTime - lastCrankEventTime) / 60000.0;

    // 计算曲柄转数增量
    cumulativeCrankRevs += round(cadenceRpm * timeDiff);

    // 更新时间戳（转换为1/1024秒单位）
    lastCrankEventTime = (currentTime * 1024) / 1000;
}

// 构建CSC数据包（符合蓝牙规范）
void buildCscPacket(uint8_t *packet)
{
    // 标志位（0x03表示同时包含轮转和踏频数据）
    packet[0] = 0x03;

    // 填充累积轮转次数（32位小端）
    memcpy(&packet[1], &cumulativeWheelRevs, 4);

    // 填充上次轮转时间（16位小端）
    memcpy(&packet[5], &lastWheelEventTime, 2);

    // 填充累积曲柄转数（16位小端）
    memcpy(&packet[7], &cumulativeCrankRevs, 2);

    // 填充上次曲柄时间（16位小端）
    memcpy(&packet[9], &lastCrankEventTime, 2);
}

void printDebugInfo(float speed, int cadence)
{
    Serial.printf("实时数据 - 速度: %.1f km/h | 踏频: %d RPM\n", speed, cadence);
    Serial.printf("轮转统计 - 总转数: %lu | 最后时间: %.3f秒\n",
                  cumulativeWheelRevs,
                  lastWheelEventTime / 1024.0);
    Serial.printf("曲柄统计 - 总转数: %u | 最后时间: %.3f秒\n\n",
                  cumulativeCrankRevs,
                  lastCrankEventTime / 1024.0);
}

void loop()
{
    // 生成模拟传感器数据（带物理合理性）
    float speedKmh = random(10, 31);  // 10-30 km/h
    int cadenceRpm = random(60, 121); // 60-120 RPM

    // 转换为物理运动参数
    updateWheelData(speedKmh);   // 更新轮转数据
    updateCrankData(cadenceRpm); // 更新曲柄数据

    // 构建符合CSC规范的数据包
    uint8_t cscData[11];
    buildCscPacket(cscData);

    // 发送数据并打印调试信息
    pCscMeasurement->setValue(cscData, sizeof(cscData));
    pCscMeasurement->notify();
    printDebugInfo(speedKmh, cadenceRpm);

    delay(2000);
}
