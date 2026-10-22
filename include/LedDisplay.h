#pragma once

#include <FastLED.h>
#include "Config.h"

enum EffectState {
    STATE_NORMAL,
    STATE_MINUTE_EFFECT,
    STATE_HOUR_EFFECT
};

class LedDisplay {
public:
    void init();
    void update(int hour, int minute, int second);
    void triggerEffect(EffectState state);
    void updateColorLive(uint32_t cHour, uint32_t cMin, uint32_t cSec); // For real-time sync
    void bootRainbow(unsigned long durationMs = 5000);

private:
    CRGB leds[NUM_LEDS];
    EffectState currentState = STATE_NORMAL;
    unsigned long effectStartTime = 0;
    
    int lastMinute = -1;
    int lastHour = -1;

    void drawNormalTime(int h, int m, int s);
    void drawTransitionEffect(int effectType, unsigned long elapsed, unsigned long duration,
                              int hour, int minute, int second);
    int mapClockValueToLed(int value);
    void setClockLed(int idx, const CRGB &color);
    int mapHourToLed(int h);
    void preserveClockEdge(int hour, int minute, int second);
};