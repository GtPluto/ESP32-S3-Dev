#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>
#include <BLE2902.h>

#include "light.h"

#define DEVICE_NAME "ESP32-S3_BLE_Demo"
#define CSC_SERVICE_UUID "00001816-0000-1000-8000-00805f9b34fb"
#define CSC_MEASUREMENT_UUID "00002a5b-0000-1000-8000-00805f9b34fb"

BLEServer *pServer;
BLECharacteristic *pCscMeasurement;

void setup()
{
    Serial.begin(115200);
    Serial.println("BLE Broadcast Started");

    LightInit();

    BLEDevice::init(DEVICE_NAME);

    pServer = BLEDevice::createServer();

    BLEService *pService = pServer->createService(CSC_SERVICE_UUID);
    pServer->setCallbacks(new LightCallbacks());
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(CSC_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinInterval(0x20); // 20ms
    pAdvertising->setMaxInterval(0x30); // 30ms
    pAdvertising->start();

    Serial.println("Device is now discoverable!");
}

void loop()
{
    LightLoop();
    delay(10); // 防止 watchdog 触发
}