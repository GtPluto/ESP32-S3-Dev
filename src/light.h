#include <Adafruit_NeoPixel.h> // WS2812 驱动库
#include <BLEServer.h>

#define LED_PIN 48  // WS2812 数据引脚
#define LED_COUNT 1 // LED数量
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

bool deviceConnected = false;

// 蓝牙连接状态回调
class LightCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        pixels.setBrightness(32); // 连接后重置亮度
    }

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        pServer->startAdvertising(); // 断开后重新广播
    }
};

void LightInit()
{
    pixels.begin();
    pixels.show();
}

void LightLoop()
{
    static unsigned long prevMillis = 0;
    unsigned long currentMillis = millis();
    if (!deviceConnected)
    { // 未连接状态：1秒红闪
        if (currentMillis - prevMillis >= 1000)
        {
            prevMillis = currentMillis;
            pixels.setBrightness(16);                         // 25%亮度 ≈ 64/255
            pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // 红色
            pixels.show();
            delay(500); // 短暂点亮
            pixels.clear();
            pixels.show();
        }
    }
    else
    {                                                                      // 已连接状态：3秒呼吸白光
        float breath = (exp(sin(millis() / 3000.0 * PI)) - 0.3678) / 2.35; // 呼吸曲线
        pixels.setBrightness(breath * 32);
        pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // 白色
        pixels.show();
    }
}