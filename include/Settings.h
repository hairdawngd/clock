#pragma once
#include <Arduino.h>

extern uint32_t colorHour;
extern uint32_t colorMinute;
extern uint32_t colorSecond;
extern uint32_t minuteEffectDuration;
extern uint32_t hourEffectDuration;
extern int gmtOffsetHours;

void loadSettings();
void saveSettings();