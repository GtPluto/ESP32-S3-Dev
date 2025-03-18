#pragma once
#include <BLEDevice.h>

// ------------ 服务 UUID ------------
#define BAT_UUID BLEUUID((uint16_t)0x180F) // 电池服务
#define DI_UUID BLEUUID((uint16_t)0x180A)  // 设备信息服务
#define CP_UUID BLEUUID((uint16_t)0x1818)  // 骑行功率服务
#define CSC_UUID BLEUUID((uint16_t)0x1816) // 速度踏频服务

// ------------ 特征 UUID ------------
// 设备信息服务特征
#define DI_SYSTEM_ID_UUID BLEUUID((uint16_t)0x2A23)     // 系统ID
#define DI_MODEL_NUMBER_UUID BLEUUID((uint16_t)0x2A24)  // 型号编号
#define DI_SERIAL_NUMBER_UUID BLEUUID((uint16_t)0x2A25) // 序列号
#define DI_FIRMWARE_REV_UUID BLEUUID((uint16_t)0x2A26)  // 固件版本
#define DI_HARDWARE_REV_UUID BLEUUID((uint16_t)0x2A27)  // 硬件版本
#define DI_SOFTWARE_REV_UUID BLEUUID((uint16_t)0x2A28)  // 软件版本
#define DI_MANUFACTURER_UUID BLEUUID((uint16_t)0x2A29)  // 制造商名称

// 其他服务特征
#define BAT_LEVEL_UUID BLEUUID((uint16_t)0x2A19)       // 电池电量
#define CSC_MEASUREMENT_UUID BLEUUID((uint16_t)0x2A5B) // CSC测量
#define CP_MEASUREMENT_UUID BLEUUID((uint16_t)0x2A63)  // 功率测量
#define CSC_FEATURE_UUID BLEUUID((uint16_t)0x2A5C)     // CSC特征
#define CP_FEATURE_UUID BLEUUID((uint16_t)0x2A65)      // CP特征
#define SENSOR_LOCATION_UUID BLEUUID((uint16_t)0x2A5D) // 传感器位置

// 描述符 UUID
#define CLIENT_CHARACTERISTIC_CONFIG_UUID BLEUUID((uint16_t)0x2902)

// ------------ 传感器位置枚举 ------------
enum SensorLocation
{
    LOC_OTHER = 0,
    LOC_REAR_WHEEL = 12
};

// ------------ 特征值属性 ------------
#define CHARACTERISTIC_PROPERTY_READ 0x02
#define CHARACTERISTIC_PROPERTY_WRITE 0x08
#define CHARACTERISTIC_PROPERTY_NOTIFY 0x10
#define CHARACTERISTIC_PROPERTY_INDICATE 0x20