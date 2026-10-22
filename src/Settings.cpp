#include "Settings.h"
#include <Preferences.h>
#include "Config.h"

uint32_t colorHour = COLOR_HOUR_DEFAULT;
uint32_t colorMinute = COLOR_MINUTE_DEFAULT;
uint32_t colorSecond = COLOR_SECOND_DEFAULT;
uint32_t colorMarker = 0x000000;
uint32_t minuteEffectDuration = MINUTE_EFFECT_DURATION_DEFAULT;
uint32_t hourEffectDuration = HOUR_EFFECT_DURATION_DEFAULT;
int gmtOffsetHours = 7;
int effectWindowStartMinute = 0;
int effectWindowEndMinute = 23 * 60 + 59;

int effectMinute2 = 1;
int effectHour2 = 2;
int brightness = BRIGHTNESS;

String wifiSSID = "";
String wifiPass = "";

Preferences prefs;

void loadSettings() {
    prefs.begin("clock", true);
    colorHour = prefs.getUInt("c_hour", COLOR_HOUR_DEFAULT);
    colorMinute = prefs.getUInt("c_min", COLOR_MINUTE_DEFAULT);
    colorSecond = prefs.getUInt("c_sec", COLOR_SECOND_DEFAULT);
    uint32_t cm = prefs.getUInt("c_mark", 0x000000);
    if (cm == 0x202020) cm = 0x000000; // default cũ sáng trắng 0x202020, khóa NVS đã lưu
    colorMarker = cm;
    minuteEffectDuration = prefs.getUInt("e_min", MINUTE_EFFECT_DURATION_DEFAULT);
    hourEffectDuration = prefs.getUInt("e_hour", HOUR_EFFECT_DURATION_DEFAULT);
    gmtOffsetHours = prefs.getInt("gmt", 7);
    effectWindowStartMinute = prefs.getInt("fx_start", 0);
    effectWindowEndMinute = prefs.getInt("fx_end", 23 * 60 + 59);
    if (effectWindowStartMinute < 0 || effectWindowStartMinute >= 24 * 60) {
        effectWindowStartMinute = 0;
    }
    if (effectWindowEndMinute < 0 || effectWindowEndMinute >= 24 * 60) {
        effectWindowEndMinute = 23 * 60 + 59;
    }
    effectMinute2 = prefs.getInt("eff_minute", 1);
    effectHour2 = prefs.getInt("eff_hour", 2);
    brightness = prefs.getInt("brightness", BRIGHTNESS);
    wifiSSID = prefs.getString("w_ssid", "");
    wifiPass = prefs.getString("w_pass", "");
    prefs.end();
}

void saveSettings() {
    prefs.begin("clock", false);
    prefs.putUInt("c_hour", colorHour);
    prefs.putUInt("c_min", colorMinute);
    prefs.putUInt("c_sec", colorSecond);
    prefs.putUInt("c_mark", colorMarker);
    prefs.putUInt("e_min", minuteEffectDuration);
    prefs.putUInt("e_hour", hourEffectDuration);
    prefs.putInt("gmt", gmtOffsetHours);
    prefs.putInt("fx_start", effectWindowStartMinute);
    prefs.putInt("fx_end", effectWindowEndMinute);
    prefs.putInt("eff_minute", effectMinute2);
    prefs.putInt("eff_hour", effectHour2);
    prefs.putInt("brightness", brightness);
    prefs.putString("w_ssid", wifiSSID);
    prefs.putString("w_pass", wifiPass);
    prefs.end();
}

bool isEffectWindowActive(int hour, int minute) {
    if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) return false;

    int currentMinute = hour * 60 + minute;
    if (effectWindowStartMinute <= effectWindowEndMinute) {
        return currentMinute >= effectWindowStartMinute &&
               currentMinute <= effectWindowEndMinute;
    }

    return currentMinute >= effectWindowStartMinute ||
           currentMinute <= effectWindowEndMinute;
}
