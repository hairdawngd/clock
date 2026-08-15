#include "Settings.h"
#include <Preferences.h>
#include "Config.h"

uint32_t colorHour = COLOR_HOUR_DEFAULT;
uint32_t colorMinute = COLOR_MINUTE_DEFAULT;
uint32_t colorSecond = COLOR_SECOND_DEFAULT;
uint32_t minuteEffectDuration = MINUTE_EFFECT_DURATION_DEFAULT;
uint32_t hourEffectDuration = HOUR_EFFECT_DURATION_DEFAULT;
int gmtOffsetHours = 7;

int effectNormal2 = 0;
int effectMinute2 = 1;
int effectHour2 = 2;
int brightnessLed1 = BRIGHTNESS;
int brightnessLed2 = BRIGHTNESS;

String wifiSSID = "";
String wifiPass = "";

Preferences prefs;

void loadSettings() {
    prefs.begin("clock", true);
    colorHour = prefs.getUInt("c_hour", COLOR_HOUR_DEFAULT);
    colorMinute = prefs.getUInt("c_min", COLOR_MINUTE_DEFAULT);
    colorSecond = prefs.getUInt("c_sec", COLOR_SECOND_DEFAULT);
    minuteEffectDuration = prefs.getUInt("e_min", MINUTE_EFFECT_DURATION_DEFAULT);
    hourEffectDuration = prefs.getUInt("e_hour", HOUR_EFFECT_DURATION_DEFAULT);
    gmtOffsetHours = prefs.getInt("gmt", 7);
    effectNormal2 = prefs.getInt("eff_n2", 0);
    effectMinute2 = prefs.getInt("eff_m2", 1);
    effectHour2 = prefs.getInt("eff_h2", 2);
    brightnessLed1 = prefs.getInt("b_led1", BRIGHTNESS);
    brightnessLed2 = prefs.getInt("b_led2", BRIGHTNESS);
    wifiSSID = prefs.getString("w_ssid", "");
    wifiPass = prefs.getString("w_pass", "");
    prefs.end();
}

void saveSettings() {
    prefs.begin("clock", false);
    prefs.putUInt("c_hour", colorHour);
    prefs.putUInt("c_min", colorMinute);
    prefs.putUInt("c_sec", colorSecond);
    prefs.putUInt("e_min", minuteEffectDuration);
    prefs.putUInt("e_hour", hourEffectDuration);
    prefs.putInt("gmt", gmtOffsetHours);
    prefs.putInt("eff_n2", effectNormal2);
    prefs.putInt("eff_m2", effectMinute2);
    prefs.putInt("eff_h2", effectHour2);
    prefs.putInt("b_led1", brightnessLed1);
    prefs.putInt("b_led2", brightnessLed2);
    prefs.putString("w_ssid", wifiSSID);
    prefs.putString("w_pass", wifiPass);
    prefs.end();
}
