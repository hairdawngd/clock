#include "TimeManager.h"
#include "Config.h"
#include <WiFi.h>
#include <time.h>

#include "Settings.h"

void TimeManager::init() {
    syncTime();
}

void TimeManager::syncTime() {
    Serial.println("Syncing time with NTP...");
    long gmtOffsetSec = gmtOffsetHours * 3600;
    
    // Xóa biến môi trường TZ cũ để tránh configTime bị ghi đè bởi cấu hình POSIX sai
    unsetenv("TZ");
    
    // configTime luôn có tác dụng lưu trữ config, kể cả khi chưa có WiFi kết nối
    configTime(gmtOffsetSec, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    
    struct tm timeinfo;
    if(getLocalTime(&timeinfo, 2000)) {
        _timeSet = true;
        _lastSyncTime = millis();
        Serial.println("Time synced from NTP.");
    } else {
        // Nếu không lấy được, kiểm tra nếu epoch time hệ thống đã hợp lệ (> 2020) thì coi như đã đồng bộ
        time_t now = time(nullptr);
        if (now > 1600000000) {
            _timeSet = true;
            _lastSyncTime = millis();
            Serial.println("System time is already valid.");
        } else {
            Serial.println("Failed to obtain time.");
        }
    }
}

void TimeManager::update() {
    unsigned long currentMillis = millis();
    // Auto sync every 30 minutes (30 * 60 * 1000 = 1800000 ms)
    if (_timeSet) {
        if (currentMillis - _lastSyncTime >= 1800000) {
            syncTime();
        }
    } else {
        // Try to sync time every 10 seconds if not set
        static unsigned long lastTry = 0;
        if (currentMillis - lastTry >= 10000) {
            lastTry = currentMillis;
            syncTime();
        }
    }
}

int TimeManager::getHour() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) return 0;
    return timeinfo.tm_hour;
}

int TimeManager::getMinute() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) return 0;
    return timeinfo.tm_min;
}

int TimeManager::getSecond() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) return 0;
    return timeinfo.tm_sec;
}

String TimeManager::getTimeString() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) return "00:00:00";
    char buffer[16];
    sprintf(buffer, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    // Serial.printf("%02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return String(buffer);
}

bool TimeManager::isTimeSet() {
    return _timeSet;
}
