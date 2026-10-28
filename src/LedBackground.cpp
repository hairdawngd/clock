#include "LedBackground.h"
#include "LedEffects.h"
#include "Settings.h"
#include "RuntimeLedController.h"

void LedBackground::init() {
    ledCount = constrain(backgroundLedCount, 1, MAX_RUNTIME_LEDS);
    Serial.printf("[LED][BG] Init: pin=%d, count=%d, enabled=%d, brightness=%d, color=#%06lX, effect=%d, window=%02d:%02d-%02d:%02d\n",
                  backgroundLedPin, ledCount, backgroundEnabled,
                  constrain(brightness, 0, 255),
                  static_cast<unsigned long>(backgroundColor & 0xFFFFFF),
                  backgroundEffect,
                  backgroundWindowStartMinute / 60,
                  backgroundWindowStartMinute % 60,
                  backgroundWindowEndMinute / 60,
                  backgroundWindowEndMinute % 60);
    controller = addRuntimeLedController(backgroundLedPin, leds, ledCount);
    if (!controller) {
        Serial.printf("[LED][BG] ERROR: controller registration failed for GPIO %d\n",
                      backgroundLedPin);
        return;
    }
    Serial.println("[LED][BG] Controller registered");
    fill_solid(leds, ledCount, CRGB::Black);
    controller->showLeds(0);
}

void LedBackground::update(int hour, int minute) {
    static unsigned long lastDebugLog = 0;
    const unsigned long now = millis();
    if (now - lastDebugLog >= 5000) {
        lastDebugLog = now;
        const bool inWindow = isBackgroundWindowActive(hour, minute);
        // Serial.printf("[LED][BG] Update: time=%02d:%02d, enabled=%d, inWindow=%d, pin=%d, count=%d, brightness=%d, color=#%06lX, effect=%d, controller=%s, output=%s\n",
        //               hour, minute, backgroundEnabled, inWindow,
        //               backgroundLedPin, ledCount,
        //               constrain(brightness, 0, 255),
        //               static_cast<unsigned long>(backgroundColor & 0xFFFFFF),
        //               backgroundEffect,
        //               controller ? "OK" : "NULL",
        //               backgroundEnabled && inWindow ? "ON" : "OFF");
    }

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
