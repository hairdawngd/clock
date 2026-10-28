#pragma once

#include <FastLED.h>

CLEDController *addRuntimeLedController(int pin, CRGB *leds, int count);
