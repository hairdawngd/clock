#pragma once

#include <FastLED.h>

void renderLedEffect(CRGB *leds, uint16_t count, int effectType,
                     unsigned long elapsed, unsigned long duration,
                     uint8_t &hue, uint16_t &chase);
