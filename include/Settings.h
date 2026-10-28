#pragma once
#include <Arduino.h>

extern uint32_t colorHour;
extern uint32_t colorMinute;
extern uint32_t colorSecond;
extern uint32_t colorMarker;
extern uint32_t minuteEffectDuration;
extern uint32_t hourEffectDuration;
extern int gmtOffsetHours;
extern int effectWindowStartMinute;
extern int effectWindowEndMinute;

extern int effectMinute2;
extern int effectHour2;
extern int brightness;
extern bool displayEnabled;
extern bool backgroundEnabled;
extern uint32_t backgroundColor;
extern int backgroundEffect;
extern uint32_t backgroundEffectDuration;
extern int backgroundWindowStartMinute;
extern int backgroundWindowEndMinute;
extern String otaPassword;
extern int displayLedPin;
extern int displayLedCount;
extern int backgroundLedPin;
extern int backgroundLedCount;
extern String mqttHost;
extern int mqttPort;
extern String mqttUser;
extern String mqttPass;
extern String mqttTopic;
extern bool mqttEnabled;
extern bool mqttDiscovery;

extern String wifiSSID;
extern String wifiPass;

void loadSettings();
void saveSettings();
bool isEffectWindowActive(int hour, int minute);
bool isBackgroundWindowActive(int hour, int minute);