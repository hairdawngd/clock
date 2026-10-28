#include "LedBackground.h"
#include "LedEffects.h"
#include "Settings.h"
#include "RuntimeLedController.h"

void LedBackground::init() {
    ledCount = constrain(backgroundLedCount, 1, MAX_RUNTIME_LEDS);
    controller = addRuntimeLedController(backgroundLedPin, leds, ledCount);
    if (!controller) {
        Serial.printf("[LED] Invalid background GPIO %d\n", backgroundLedPin);
        return;
    }
    fill_solid(leds, ledCount, CRGB::Black);
    controller->showLeds(0);
}

void LedBackground::update(int hour, int minute) {
    if (!backgroundEnabled) {
        fill_solid(leds, ledCount, CRGB::Black);
        if (controller) controller->showLeds(constrain(brightness, 0, 255));
        return;
    }

    bool inWindow = isBackgroundWindowActive(hour, minute);
    if (!inWindow) {
        fill_solid(leds, ledCount, CRGB::Black);
        if (controller) controller->showLeds(constrain(brightness, 0, 255));
        effectActive = false;
        return;
    }

    if (lastMinute != minute || !effectActive) {
        lastMinute = minute;
        effectStartTime = millis();
        effectActive = backgroundEffect != 0;
    }

    if (backgroundEffect == 0) {
        fill_solid(leds, ledCount, CRGB(backgroundColor));
    } else if (backgroundEffect != 0) {
        unsigned long elapsed = millis() - effectStartTime;
        if (elapsed >= backgroundEffectDuration) {
            effectStartTime = millis();
            elapsed = 0;
        }
        renderLedEffect(leds, ledCount, backgroundEffect,
                        elapsed, backgroundEffectDuration, hue, chase);
    }
    if (controller) controller->showLeds(constrain(brightness, 0, 255));
}
