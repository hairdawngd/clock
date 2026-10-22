#include "LedDisplay.h"
#include "Settings.h"

void LedDisplay::init() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(255);
    FastLED.clear();
    FastLED.show();
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
    if (i >= 0 && i < NUM_LEDS) leds[i] = color;
    // Mirror physical LED 60 (index EFFECT_START_INDEX-1) to follow LED 1 (index 0)
    if (i == 0) {
        int mirror = EFFECT_START_INDEX - 1;
        if (mirror >= 0 && mirror < NUM_LEDS) leds[mirror] = color;
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

    nscale8_video(leds, NUM_LEDS, constrain(brightness, 0, 255));
    if (currentState == STATE_NORMAL) {
        preserveClockEdge(hour, minute, second);   // chỉ mirror LED 60 khi hiển thị giờ bình thường
    }
    FastLED.show();
}


void LedDisplay::preserveClockEdge(int hour, int minute, int second) {
    int mirrorIndex = EFFECT_START_INDEX - 1; // physical LED 60
    if (mirrorIndex < 0 || mirrorIndex >= NUM_LEDS) return;

    int s = mapClockValueToLed(second);
    int m = mapClockValueToLed(minute);
    int h = mapHourToLed(hour);

    // Mirror whatever is currently at LED index 0 (physical LED 1)
    if (0 >= 0 && 0 < NUM_LEDS) leds[mirrorIndex] = leds[0];
}

void LedDisplay::drawNormalTime(int h, int m, int s) {
    m = mapClockValueToLed(m);
    s = mapClockValueToLed(s);

    for (int i = 0; i < CLOCK_LED_COUNT && i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }
    // Also clear the mirror LED (physical 60 -> index EFFECT_START_INDEX-1)
    int mirrorIndex = EFFECT_START_INDEX - 1;
    if (mirrorIndex >= 0 && mirrorIndex < NUM_LEDS) leds[mirrorIndex] = CRGB::Black;

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
    static uint8_t hue = 0;
    static uint16_t chase = 0;
    hue++;
    chase++;

    switch (effectType) {
        case 0:
            fill_solid(leds, NUM_LEDS, CRGB::Black);
            drawNormalTime(hour, minute, second);
            break;
        case 1:
            fill_rainbow(leds, NUM_LEDS, hue, 255 / NUM_LEDS + 1);
            break;
        case 2:
            fadeToBlackBy(leds, NUM_LEDS, 20);
            if (random8() < 120) {
                leds[random16(NUM_LEDS)] += CHSV(hue, 255, 255);
            }
            break;
        case 3: {
            uint8_t brightnessValue = beatsin8(30, 0, 255);
            fill_solid(leds, NUM_LEDS, CHSV(hue, 255, brightnessValue));
            break;
        }
        case 4:
            for (int i = 0; i < NUM_LEDS; i++) {
                if (random8() < 5) leds[i] = CHSV(random8(), 255, 255);
                else leds[i].nscale8(250);
            }
            break;
        case 5:
            if ((millis() / 250) % 2) {
                fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255));
            } else {
                fill_solid(leds, NUM_LEDS, CRGB::Black);
            }
            break;
        case 6:
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = HeatColor(random8(80, 255));
                leds[i].fadeToBlackBy(random8(0, 120));
            }
            break;
        case 7:
            fadeToBlackBy(leds, NUM_LEDS, 80);
            leds[chase % NUM_LEDS] = CHSV(hue, 255, 255);
            leds[(chase + NUM_LEDS / 3) % NUM_LEDS] = CHSV(hue + 85, 255, 255);
            leds[(chase + (NUM_LEDS * 2) / 3) % NUM_LEDS] = CHSV(hue + 170, 255, 255);
            break;
        case 8:
            fadeToBlackBy(leds, NUM_LEDS, 35);
            if (random8() < 160) {
                leds[random16(NUM_LEDS)] += CRGB::White;
            }
            break;
        case 9: {
            unsigned long sweepMs = duration * 2 / 5;
            if (sweepMs == 0) sweepMs = 1;
            if (elapsed < sweepMs) {
                int head = (int)((elapsed * NUM_LEDS) / sweepMs);
                if (head >= NUM_LEDS) head = NUM_LEDS - 1;
                for (int i = 0; i < NUM_LEDS; i++) {
                    if (i <= head) {
                        leds[i] = CHSV((i * 255) / NUM_LEDS, 255, 255);
                    } else {
                        leds[i] = CRGB::Black;
                    }
                }
            } else {
                unsigned long fadeMs = duration > sweepMs ? duration - sweepMs : 1;
                unsigned long fadeTime = elapsed - sweepMs;
                if (fadeTime > fadeMs) fadeTime = fadeMs;
                uint8_t value = 255 - (uint8_t)((fadeTime * 255) / fadeMs);
                value = scale8(value, value);
                fill_solid(leds, NUM_LEDS, CRGB(value, value, value));
            }
            break;
        }
        default:
            fill_solid(leds, NUM_LEDS, CRGB::Black);
            drawNormalTime(hour, minute, second);
            break;
    }
}

void LedDisplay::bootRainbow(unsigned long durationMs) {
    uint8_t hue = 0;
    unsigned long start = millis();

    while (millis() - start < durationMs) {
        fill_rainbow(leds, NUM_LEDS, hue, 255 / NUM_LEDS + 1);
        hue += 3;

        FastLED.show();
        delay(20);
    }

    FastLED.clear();
    FastLED.show();
}
