#include "Settings.h"
#include <Preferences.h>
#include "Config.h"

uint32_t colorHour = COLOR_HOUR_DEFAULT;
uint32_t colorMinute = COLOR_MINUTE_DEFAULT;
uint32_t colorSecond = COLOR_SECOND_DEFAULT;
uint32_t minuteEffectDuration = MINUTE_EFFECT_DURATION_DEFAULT;
uint32_t hourEffectDuration = HOUR_EFFECT_DURATION_DEFAULT;
int gmtOffsetHours = 7;

Preferences prefs;

void loadSettings() {
    prefs.begin("clock", true);
    colorHour = prefs.getUInt("c_hour", COLOR_HOUR_DEFAULT);
    colorMinute = prefs.getUInt("c_min", COLOR_MINUTE_DEFAULT);
    colorSecond = prefs.getUInt("c_sec", COLOR_SECOND_DEFAULT);
    minuteEffectDuration = prefs.getUInt("e_min", MINUTE_EFFECT_DURATION_DEFAULT);
    hourEffectDuration = prefs.getUInt("e_hour", HOUR_EFFECT_DURATION_DEFAULT);
    gmtOffsetHours = prefs.getInt("gmt", 7);
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
    prefs.end();
}
