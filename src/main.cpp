#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "TimeManager.h"
#include "LedDisplay.h"
#include "Settings.h"
#include "WebUI.h"
#include <SPIFFS.h>

TimeManager timeManager;
LedDisplay display;

unsigned long lastDrawTime = 0;
const int DRAW_INTERVAL = 30;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Starting LED Clock...");

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed");
    }

    loadSettings();
    display.init();
    display.bootRainbow(5000); 

    if (wifiSSID.length() > 0) {
        WiFi.mode(WIFI_STA);
        WiFi.setHostname("clock-led");
        WiFi.setSleep(false);
        WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());
        unsigned long start = millis();
        bool ok = false;
        while (millis() - start < 15000) {
            if (WiFi.status() == WL_CONNECTED) {
                ok = true;
                break;
            }
            delay(200);
        }
        if (!ok) {
            Serial.println("[WIFI] STA kết nối thất bại. Bật AP.");
            WiFi.disconnect();
            WiFi.mode(WIFI_AP);
            WiFi.softAP("LED Clock", "12345678");
        } else {
            Serial.printf("[WIFI] STA OK: IP %s\n", WiFi.localIP().toString().c_str());
        }
    } else {
        WiFi.mode(WIFI_AP);
        WiFi.setHostname("clock-led");
        WiFi.setSleep(false);
        WiFi.softAP("LED Clock", "12345678");
        Serial.printf("[WIFI] AP OK: IP %s\n", WiFi.softAPIP().toString().c_str());
    }

    setupWebUI();
    Serial.printf("[SERVER] Web server started. STA IP: %s, AP IP: %s\n",
                  WiFi.localIP().toString().c_str(),
                  WiFi.softAPIP().toString().c_str());

    timeManager.init();
}

void loop() {
    unsigned long currentMillis = millis();
    timeManager.update();

    if (currentMillis - lastDrawTime >= DRAW_INTERVAL) {
        lastDrawTime = currentMillis;

        if (timeManager.isTimeSet()) {
            display.update(
                timeManager.getHour(),
                timeManager.getMinute(),
                timeManager.getSecond()
            );
        } else {
            display.update(0, 0, 0);
        }
    }
}