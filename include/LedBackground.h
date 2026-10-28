#pragma once

#include <FastLED.h>
#include "Config.h"

class LedBackground {
public:
    void init();
    void update(int hour, int minute);

private:
    CRGB leds[MAX_RUNTIME_LEDS];
    CLEDController *controller = nullptr;
    int ledCount = NUM_BACKGROUND_LEDS;
    unsigned long effectStartTime = 0;
    int lastMinute = -1;
    bool effectActive = false;
    uint8_t hue = 0;
    uint16_t chase = 0;
};
