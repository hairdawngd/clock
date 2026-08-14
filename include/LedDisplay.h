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

private:
    CRGB leds[NUM_LEDS];
    EffectState currentState = STATE_NORMAL;
    unsigned long effectStartTime = 0;
    
    int lastMinute = -1;
    int lastHour = -1;

    void drawNormalTime(int h, int m, int s);
    void drawMinuteEffect();
    void drawHourEffect();
    
    int mapHourToLed(int h);
};