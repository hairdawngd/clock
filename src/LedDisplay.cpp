#include "LedDisplay.h"
#include "LedEffects.h"
#include "Settings.h"
#include "RuntimeLedController.h"

void LedDisplay::init() {
    ledCount = constrain(displayLedCount, 1, MAX_RUNTIME_LEDS);
    controller = addRuntimeLedController(displayLedPin, leds, ledCount);
    if (!controller) {
        Serial.printf("[LED] Invalid display GPIO %d\n", displayLedPin);
        return;
    }
    FastLED.setBrightness(constrain(brightness, 0, 255));
    fill_solid(leds, ledCount, CRGB::Black);
    controller->showLeds(0);
}

void LedDisplay::updateColorLive(uint32_t cHour, uint32_t cMin, uint32_t cSec) {
    colorHour = cHour;
    colorMinute = cMin;
    colorSecond = cSec;
}

int LedDisplay::mapClockValueToLed(int value) {
    value %= CLOCK_LED_COUNT;
    if (value < 0) value += CLOCK_LED_COUNT;
    return value;
}

void LedDisplay::setClockLed(int idx, const CRGB &color) {
    int i = mapClockValueToLed(idx);
    if (i >= 0 && i < ledCount) leds[i] = color;
    // Mirror physical LED 60 (index EFFECT_START_INDEX-1) to follow LED 1 (index 0)
    if (i == 0) {
        int mirror = ledCount > CLOCK_LED_COUNT ? CLOCK_LED_COUNT : -1;
        if (mirror >= 0 && mirror < ledCount) leds[mirror] = color;
    }
}

int LedDisplay::mapHourToLed(int h) {
    int hour12 = h % 12;
    if (hour12 < 0) hour12 += 12;
    return mapClockValueToLed(hour12 * 5);
}

void LedDisplay::triggerEffect(EffectState state) {
    currentState = state;
    effectStartTime = millis();
}

void LedDisplay::update(int hour, int minute, int second) {
    unsigned long currentMillis = millis();

    if (!displayEnabled) {
        fill_solid(leds, ledCount, CRGB::Black);
        if (controller) controller->showLeds(constrain(brightness, 0, 255));
        return;
    }

    // Check for transitions
    if (lastMinute != -1 && minute != lastMinute) {
        EffectState nextState;
        if (hour != lastHour && lastHour != -1) {
            nextState = STATE_HOUR_EFFECT;
        } else {
            nextState = STATE_MINUTE_EFFECT;
        }

        int selectedEffect = nextState == STATE_HOUR_EFFECT ? effectHour2 : effectMinute2;
        if (selectedEffect != 0 && isEffectWindowActive(hour, minute)) {
            currentState = nextState;
            effectStartTime = currentMillis;
            FastLED.clear();
        } else {
            currentState = STATE_NORMAL;
        }
    }

    lastMinute = minute;
    lastHour = hour;

    // Check effect timeout
    unsigned long effectDuration = currentState == STATE_MINUTE_EFFECT
        ? minuteEffectDuration
        : hourEffectDuration;
    int selectedEffect = currentState == STATE_MINUTE_EFFECT
        ? effectMinute2
        : effectHour2;
    if ((currentState == STATE_MINUTE_EFFECT || currentState == STATE_HOUR_EFFECT) &&
        selectedEffect == 0) {
        currentState = STATE_NORMAL;
        FastLED.clear();
    }
    if ((currentState == STATE_MINUTE_EFFECT || currentState == STATE_HOUR_EFFECT) &&
        (currentMillis - effectStartTime >= effectDuration)) {
        currentState = STATE_NORMAL;
        FastLED.clear();
    }
    if ((currentState == STATE_MINUTE_EFFECT || currentState == STATE_HOUR_EFFECT) &&
        !isEffectWindowActive(hour, minute)) {
        currentState = STATE_NORMAL;
        FastLED.clear();
    }

    // Draw based on state
    switch (currentState) {
        case STATE_NORMAL:
            drawNormalTime(hour, minute, second);
            break;
        case STATE_MINUTE_EFFECT:
            drawTransitionEffect(effectMinute2, currentMillis - effectStartTime,
                                 minuteEffectDuration, hour, minute, second);
            break;
        case STATE_HOUR_EFFECT:
            drawTransitionEffect(effectHour2, currentMillis - effectStartTime,
                                 hourEffectDuration, hour, minute, second);
            break;
    }

    FastLED.setBrightness(constrain(brightness, 0, 255));
    if (currentState == STATE_NORMAL) {
        preserveClockEdge(hour, minute, second);   // chỉ mirror LED 60 khi hiển thị giờ bình thường
    }
    if (controller) controller->showLeds(constrain(brightness, 0, 255));
}


void LedDisplay::preserveClockEdge(int hour, int minute, int second) {
    int mirrorIndex = ledCount > CLOCK_LED_COUNT ? CLOCK_LED_COUNT : -1;
    if (mirrorIndex < 0 || mirrorIndex >= ledCount) return;

    int s = mapClockValueToLed(second);
    int m = mapClockValueToLed(minute);
    int h = mapHourToLed(hour);

    // Mirror whatever is currently at LED index 0 (physical LED 1)
    if (0 >= 0 && 0 < ledCount) leds[mirrorIndex] = leds[0];
}

void LedDisplay::drawNormalTime(int h, int m, int s) {
    m = mapClockValueToLed(m);
    s = mapClockValueToLed(s);

    int clockCount = min(CLOCK_LED_COUNT, ledCount);
    for (int i = 0; i < clockCount; i++) {
        leds[i] = CRGB::Black;
    }
    // Also clear the mirror LED (physical 60 -> index EFFECT_START_INDEX-1)
    int mirrorIndex = ledCount > CLOCK_LED_COUNT ? CLOCK_LED_COUNT : -1;
    if (mirrorIndex >= 0 && mirrorIndex < ledCount) leds[mirrorIndex] = CRGB::Black;

    // Fixed clock markers: 12, 3, 6, 9. Draw first so hands can overlap them.
    const int markers[] = {0, 15, 30, 45};
    for (int marker : markers) {
        setClockLed(marker, colorMarker);
    }

    // led[0] is physical LED 1 and clock position 12.
    // Vẽ giờ sau cùng để kim giờ không bị giây/phút đè (giây quét mọi vị trí mỗi phút).
    setClockLed(s, colorSecond);
    setClockLed(m, colorMinute);
    setClockLed(mapHourToLed(h), colorHour);
}


void LedDisplay::drawTransitionEffect(int effectType, unsigned long elapsed, unsigned long duration,
                                      int hour, int minute, int second) {
    if (effectType == 0) {
        drawNormalTime(hour, minute, second);
        return;
    }
    renderLedEffect(leds, ledCount, effectType, elapsed, duration,
                    effectHue, effectChase);
}

void LedDisplay::bootRainbow(unsigned long durationMs) {
    uint8_t hue = 0;
    unsigned long start = millis();

    while (millis() - start < durationMs) {
        fill_rainbow(leds, ledCount, hue, 255 / ledCount + 1);
        hue += 3;

        controller->showLeds(constrain(brightness, 0, 255));
        delay(20);
    }

    fill_solid(leds, ledCount, CRGB::Black);
    controller->showLeds(0);
}
