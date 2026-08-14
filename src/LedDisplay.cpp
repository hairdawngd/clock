#include "LedDisplay.h"
#include "Settings.h"

void LedDisplay::init() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
}

int LedDisplay::mapHourToLed(int h) {
    // 12 hour format (0-11)
    int hour12 = h % 12;
    // Map 0-11 to 0-59 (each hour is 5 LEDs apart)
    return (hour12 * 5) % NUM_LEDS;
}

void LedDisplay::triggerEffect(EffectState state) {
    currentState = state;
    effectStartTime = millis();
}

void LedDisplay::update(int hour, int minute, int second) {
    unsigned long currentMillis = millis();

    // Check for transitions
    if (lastMinute != -1 && minute != lastMinute) {
        if (hour != lastHour && lastHour != -1) {
            currentState = STATE_HOUR_EFFECT;
        } else {
            currentState = STATE_MINUTE_EFFECT;
        }
        effectStartTime = currentMillis;
    }
    
    lastMinute = minute;
    lastHour = hour;

    // Check effect timeout
    if (currentState == STATE_MINUTE_EFFECT && (currentMillis - effectStartTime >= minuteEffectDuration)) {
        currentState = STATE_NORMAL;
    } else if (currentState == STATE_HOUR_EFFECT && (currentMillis - effectStartTime >= hourEffectDuration)) {
        currentState = STATE_NORMAL;
    }

    // Draw based on state
    switch (currentState) {
        case STATE_NORMAL:
            drawNormalTime(hour, minute, second);
            break;
        case STATE_MINUTE_EFFECT:
            drawMinuteEffect();
            break;
        case STATE_HOUR_EFFECT:
            drawHourEffect();
            break;
    }
    
    FastLED.show();
}

void LedDisplay::drawNormalTime(int h, int m, int s) {
    // Clear and draw background markings
    for (int i = 0; i < NUM_LEDS; i++) {
        // Fix 4 points: 0, 15, 30, 45 (always on, maybe different color/brightness if needed)
        if (i == 0 || i == 15 || i == 30 || i == 45) {
            leds[i] = CRGB(0x20, 0x20, 0x20); // Slightly brighter for main marks
        } else {
            leds[i] = CRGB::Black;
        }
    }

    // Draw Hands
    int hourLed = mapHourToLed(h);
    
    // Mix colors if they overlap
    
    // Hour
    leds[hourLed] = colorHour;
    
    // Minute
    if (leds[m] == (CRGB)colorHour) {
        // Overlap Hour + Minute (e.g. Yellow-ish)
        leds[m] = CRGB(0xFF, 0xFF, 0x00); 
    } else {
        leds[m] = colorMinute;
    }

    // Second
    if (leds[s] == (CRGB)colorHour) {
        leds[s] = CRGB(0xFF, 0x00, 0xFF); // Hour + Sec = Magenta
    } else if (leds[s] == (CRGB)colorMinute) {
        leds[s] = CRGB(0x00, 0xFF, 0xFF); // Min + Sec = Cyan
    } else if (leds[s] == CRGB(0xFF, 0xFF, 0x00)) {
        leds[s] = CRGB(0xFF, 0xFF, 0xFF); // All three overlap = White
    } else {
        leds[s] = colorSecond;
    }
}

void LedDisplay::drawMinuteEffect() {
    // Random color fade effect
    // Generate a random target color once per effect start or smoothly transition
    // For simplicity, fill with random fading colors
    static uint8_t hue = 0;
    hue += 5; // Rotate hue fast
    
    fadeToBlackBy(leds, NUM_LEDS, 20); // Fade existing
    
    // Add new random sparks
    if (random8() < 120) {
        int pos = random16(NUM_LEDS);
        leds[pos] += CHSV(random8(), 255, 255);
    }
}

void LedDisplay::drawHourEffect() {
    // Rainbow fade effect
    static uint8_t startIndex = 0;
    startIndex += 2; // Speed of rainbow rotation
    
    fadeToBlackBy(leds, NUM_LEDS, 20); // Fade existing
    
    // Draw rainbow on top of fading background
    CRGB rainbow[NUM_LEDS];
    fill_rainbow(rainbow, NUM_LEDS, startIndex, 255 / NUM_LEDS);
    
    // Blend rainbow into leds
    for(int i = 0; i < NUM_LEDS; i++) {
        // only draw some pixels to let fade show, or just draw rainbow directly but faded
        if (random8() < 50) {
           leds[i] = rainbow[i];
        }
    }
}
