#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BikeData.h"
#include "BatteryService.h"
#include "CSCService.h"
#include "CPService.h"
#include "DeviceInfoService.h"

// LED 引脚定义
#define LED_PIN 2

BikeData bikeData;
BLEServer *pServer;
BatteryService *pBatteryService;
CSCService *pCSCService;
CPService *pCPService;
DeviceInfoService *pDeviceInfoService;

// 连接状态回调
class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("设备已连接");
    }
    void onDisconnect(BLEServer *pServer)
    {
        digitalWrite(LED_PIN, LOW);
        Serial.println("设备已断开");
        pServer->startAdvertising(); // 重新开始广播
        Serial.println("重新开始广播");
    }
};

void setupBLE()
{
    BLEDevice::init("Keiser M to GATT");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // 创建所有服务实例
    pBatteryService = new BatteryService(pServer);
    pCSCService = new CSCService(pServer);
    pCPService = new CPService(pServer);
    pDeviceInfoService = new DeviceInfoService(pServer);

    // 启动服务
    pServer->getAdvertising()->addServiceUUID(BAT_UUID);
    pServer->getAdvertising()->addServiceUUID(CSC_UUID);
    pServer->getAdvertising()->addServiceUUID(CP_UUID);
    pServer->getAdvertising()->setAppearance(0x0480); // Cycling appearance
    pServer->getAdvertising()->start();

    Serial.println("BLE 服务已启动");
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    setupBLE();
    Serial.println("初始化完成");
}

void loop()
{
    bikeData.update();
    auto data = bikeData.getData();

    // 更新所有传感器数据
    pCSCService->updateMeasurement(
        data.wheel_rev,
        data.w_event_time,
        data.crank_rev,
        data.c_event_time);

    pCPService->updateMeasurement(data.power);

    // 打印调试信息
    Serial.printf("数据更新: 功率=%dW, 轮转=%d, 踏频=%d\n",
                  data.power, data.wheel_rev, data.crank_rev);

    delay(100); // 10Hz 更新频率
}