#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "Config.h"
#include "TimeManager.h"
#include "LedDisplay.h"
#include "Settings.h"
#include "WebUI.h"

TimeManager timeManager;
LedDisplay display;

unsigned long lastDrawTime = 0;
const int DRAW_INTERVAL = 30; // ~33fps for smooth effects

bool enterWiFiSetup = false;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Starting LED Clock...");

    loadSettings();
    display.init();

    // Set WiFi to Station & AP mode so we can serve UI without forcing portal immediately
    WiFi.mode(WIFI_AP_STA);
    if (WiFi.SSID() != "") {
        WiFi.begin();
    } else {
        WiFi.softAP("LED Clock", "12345678");
    }

    timeManager.init();
    setupWebUI();
}

void loop() {
    unsigned long currentMillis = millis();
    timeManager.update();
    
    if (enterWiFiSetup) {
        enterWiFiSetup = false;
        // Stop custom web server on port 80 to avoid conflicts with WiFiManager
        stopWebUI();
        
        WiFiManager wifiManager;
        wifiManager.startConfigPortal("LED Clock", "12345678");
        
        // Reboot after exiting portal to apply new WiFi or restart standard WebUI
        ESP.restart();
    }

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