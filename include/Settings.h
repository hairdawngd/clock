#pragma once
#include <Arduino.h>

extern uint32_t colorHour;
extern uint32_t colorMinute;
extern uint32_t colorSecond;
extern uint32_t colorMarker;
extern uint32_t minuteEffectDuration;
extern uint32_t hourEffectDuration;
extern int gmtOffsetHours;

extern int effectNormal2;
extern int effectMinute2;
extern int effectHour2;
extern int brightness;

extern String wifiSSID;
extern String wifiPass;

void loadSettings();
void saveSettings();