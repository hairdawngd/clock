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
    void drawEffect2(int effectType);
    void updateColorLive(uint32_t cHour, uint32_t cMin, uint32_t cSec); // For real-time sync

private:
    CRGB leds[NUM_LEDS_1];
    CRGB leds2[NUM_LEDS_2]; // LED strip 2
    EffectState currentState = STATE_NORMAL;
    unsigned long effectStartTime = 0;
    
    int lastMinute = -1;
    int lastHour = -1;

    void drawNormalTime(int h, int m, int s);
    void drawMinuteEffect();
    void drawHourEffect();
    
    int mapHourToLed(int h);
};