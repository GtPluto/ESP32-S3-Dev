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

// 调试标志
#define DEBUG_MEMORY true
#define DEBUG_BLE true

// 全局变量，用于标记是否发生异常
volatile bool hadException = false;

// 计时器变量，用于监测系统是否卡住
unsigned long lastActiveTime = 0;
const unsigned long WATCHDOG_TIMEOUT = 10000; // 10秒超时

BikeData bikeData;
BLEServer *pServer = nullptr;
BatteryService *pBatteryService = nullptr;
CSCService *pCSCService = nullptr;
CPService *pCPService = nullptr;
DeviceInfoService *pDeviceInfoService = nullptr;

// 连接状态回调
class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        digitalWrite(LED_PIN, HIGH);
        if (DEBUG_BLE)
            Serial.println("[BLE] 设备已连接");
    }
    void onDisconnect(BLEServer *pServer)
    {
        digitalWrite(LED_PIN, LOW);
        if (DEBUG_BLE)
            Serial.println("[BLE] 设备已断开");
        if (pServer)
        {
            pServer->startAdvertising();
            if (DEBUG_BLE)
                Serial.println("[BLE] 重新开始广播");
        }
    }
};

bool setupBLE()
{
    try
    {
        if (DEBUG_BLE)
            Serial.println("[BLE] 初始化BLE设备...");
        BLEDevice::init("Indoor Bike");

        if (DEBUG_BLE)
            Serial.println("[BLE] 创建BLE服务器...");
        pServer = BLEDevice::createServer();
        if (!pServer)
        {
            Serial.println("[ERROR] 创建BLE服务器失败");
            return false;
        }

        pServer->setCallbacks(new ServerCallbacks());

        // 创建所有服务实例
        if (DEBUG_BLE)
            Serial.println("[BLE] 创建服务实例...");

        if (DEBUG_MEMORY)
        {
            Serial.printf("[MEM] Free heap before services: %d\n", ESP.getFreeHeap());
        }

        pBatteryService = new BatteryService(pServer);
        if (!pBatteryService)
        {
            Serial.println("[ERROR] 创建电池服务失败");
            return false;
        }

        pCSCService = new CSCService(pServer);
        if (!pCSCService)
        {
            Serial.println("[ERROR] 创建CSC服务失败");
            return false;
        }

        pCPService = new CPService(pServer);
        if (!pCPService)
        {
            Serial.println("[ERROR] 创建CP服务失败");
            return false;
        }

        pDeviceInfoService = new DeviceInfoService(pServer);
        if (!pDeviceInfoService)
        {
            Serial.println("[ERROR] 创建设备信息服务失败");
            return false;
        }

        if (DEBUG_MEMORY)
        {
            Serial.printf("[MEM] Free heap after services: %d\n", ESP.getFreeHeap());
        }

        // 启动服务
        if (DEBUG_BLE)
            Serial.println("[BLE] 启动广播...");
        auto advertising = pServer->getAdvertising();
        if (!advertising)
        {
            Serial.println("[ERROR] 获取广播对象失败");
            return false;
        }

        advertising->addServiceUUID(BAT_UUID);
        advertising->addServiceUUID(CSC_UUID);
        advertising->addServiceUUID(CP_UUID);
        advertising->setAppearance(0x0480); // Cycling appearance
        advertising->start();

        if (DEBUG_BLE)
            Serial.println("[BLE] BLE服务已启动");
        return true;
    }
    catch (const std::exception &e)
    {
        Serial.printf("[ERROR] BLE设置失败: %s\n", e.what());
        return false;
    }
    catch (...)
    {
        Serial.println("[ERROR] BLE设置失败: 未知错误");
        return false;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n[INIT] 系统启动...");
    if (DEBUG_MEMORY)
    {
        Serial.printf("[MEM] Initial free heap: %d\n", ESP.getFreeHeap());
    }

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // 初始化看门狗计时器
    lastActiveTime = millis();

    // 使用时间戳作为随机数种子
    randomSeed(millis());

    // 设置BLE
    if (!setupBLE())
    {
        Serial.println("[ERROR] BLE初始化失败，系统重启");
        delay(3000);
        ESP.restart();
    }

    Serial.println("[INIT] 初始化完成");
}

void loop()
{
    static uint32_t lastHeapCheck = 0;
    unsigned long currentTime = millis();

    // 定期检查堆内存
    if (DEBUG_MEMORY && (currentTime - lastHeapCheck > 5000))
    {
        Serial.printf("[MEM] Free heap: %d\n", ESP.getFreeHeap());
        lastHeapCheck = currentTime;
    }

    // 更新数据
    bikeData.update();
    auto data = bikeData.getData();

    // 更新传感器数据
    try
    {
        // 检查必要的指针
        if (!pServer || !pCSCService || !pCPService)
        {
            Serial.println("[ERROR] 检测到无效的服务指针，重新初始化...");
            if (!setupBLE())
            {
                Serial.println("[ERROR] 重新初始化失败，系统重启");
                delay(1000);
                ESP.restart();
            }
            return;
        }

        // 更新CSC服务
        pCSCService->updateMeasurement(
            data.wheel_rev,
            data.w_event_time,
            data.crank_rev,
            data.c_event_time);

        // 更新CP服务
        pCPService->updateMeasurement(data.power);

        // 数据更新成功，重置看门狗计时器
        lastActiveTime = currentTime;
    }
    catch (const std::exception &e)
    {
        Serial.printf("[ERROR] 更新传感器数据失败: %s\n", e.what());
    }
    catch (...)
    {
        Serial.println("[ERROR] 更新传感器数据时发生未知错误");
    }

    // 添加短暂延时以防止过于频繁的更新
    delay(50);

    // 看门狗检查
    if (currentTime - lastActiveTime > WATCHDOG_TIMEOUT)
    {
        Serial.println("[ERROR] 系统卡住检测到，准备重启...");
        delay(1000);
        ESP.restart();
    }
}