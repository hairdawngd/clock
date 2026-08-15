#include "LedDisplay.h"
#include "Settings.h"

void LedDisplay::init() {
    FastLED.addLeds<LED_TYPE, LED_PIN_1, COLOR_ORDER>(leds, NUM_LEDS_1).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS_2).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(255);
    FastLED.clear();
    FastLED.show();
}

void LedDisplay::updateColorLive(uint32_t cHour, uint32_t cMin, uint32_t cSec) {
    colorHour = cHour;
    colorMinute = cMin;
    colorSecond = cSec;
}

int LedDisplay::mapHourToLed(int h) {
    // 12 hour format (0-11)
    int hour12 = h % 12;
    // Map 0-11 to 0-59 (each hour is 5 LEDs apart)
    return (hour12 * 5) % NUM_LEDS_1;
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
            drawEffect2(effectNormal2);
            break;
        case STATE_MINUTE_EFFECT:
            drawMinuteEffect();
            drawEffect2(effectMinute2);
            break;
        case STATE_HOUR_EFFECT:
            drawHourEffect();
            drawEffect2(effectHour2);
            break;
    }

    nscale8_video(leds, NUM_LEDS_1, constrain(brightnessLed1, 0, 255));
    nscale8_video(leds2, NUM_LEDS_2, constrain(brightnessLed2, 0, 255));
    FastLED.show();
}

// ================ Effect Drawing Functions by led strip 1=====================
void LedDisplay::drawNormalTime(int h, int m, int s) {
    // Clear and draw background markings
    for (int i = 0; i < NUM_LEDS_1; i++) {
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


// ================ Effect Drawing Functions by led strip 2=====================
void LedDisplay::drawMinuteEffect() {
    // Random color fade effect
    // Generate a random target color once per effect start or smoothly transition
    // For simplicity, fill with random fading colors
    static uint8_t hue = 0;
    hue += 5; // Rotate hue fast
    
    fadeToBlackBy(leds, NUM_LEDS_2, 20); // Fade existing
    
    // Add new random sparks
    if (random8() < 120) {
        int pos = random16(NUM_LEDS_2);
        leds[pos] += CHSV(random8(), 255, 255);
    }
}

void LedDisplay::drawHourEffect() {
    // Rainbow fade effect
    static uint8_t startIndex = 0;
    startIndex += 2; // Speed of rainbow rotation
    
    fadeToBlackBy(leds, NUM_LEDS_2, 20); // Fade existing
    
    // Draw rainbow on top of fading background
    CRGB rainbow[NUM_LEDS_2];
    fill_rainbow(rainbow, NUM_LEDS_2, startIndex, 255 / NUM_LEDS_2);
    
    // Blend rainbow into leds
    for(int i = 0; i < NUM_LEDS_2; i++) {
        // only draw some pixels to let fade show, or just draw rainbow directly but faded
        if (random8() < 50) {
           leds[i] = rainbow[i];
        }
    }
}

void LedDisplay::drawEffect2(int effectType) {
    // effectType: 0=Tắt, 1=Cầu vồng, 2=Sao băng, 3=Nhịp thở, 4=Đổi màu ngẫu nhiên, 5=Chớp nháy, 6=Lửa, 7=Đuổi màu, 8=Lấp lánh
    static uint8_t hue = 0;
    static uint16_t chase = 0;
    hue++;
    chase++;
    
    switch (effectType) {
        case 0: // Tắt
            fill_solid(leds2, NUM_LEDS_2, CRGB::Black);
            break;
            
        case 1: // Cầu vồng
            fill_rainbow(leds2, NUM_LEDS_2, hue, 255 / NUM_LEDS_2);
            break;
            
        case 2: // Sao băng
            fadeToBlackBy(leds2, NUM_LEDS_2, 20);
            if (random8() < 120) {
                leds2[random16(NUM_LEDS_2)] += CHSV(hue, 255, 255);
            }
            break;
            
        case 3: // Nhịp thở
        {
            uint8_t b = beatsin8(30, 0, 255);
            fill_solid(leds2, NUM_LEDS_2, CHSV(hue, 255, b));
            break;
        }
        case 4: // Đổi màu ngẫu nhiên (chớp ngẫu nhiên)
            for(int i=0; i<NUM_LEDS_2; i++) {
                if(random8() < 5) leds2[i] = CHSV(random8(), 255, 255);
                else leds2[i].nscale8(250);
            }
            break;

        case 5: // Chớp nháy
            if ((millis() / 250) % 2) {
                fill_solid(leds2, NUM_LEDS_2, CHSV(hue, 255, 255));
            } else {
                fill_solid(leds2, NUM_LEDS_2, CRGB::Black);
            }
            break;

        case 6: // Lửa
            for (int i = 0; i < NUM_LEDS_2; i++) {
                leds2[i] = HeatColor(random8(80, 255));
                leds2[i].fadeToBlackBy(random8(0, 120));
            }
            break;

        case 7: // Đuổi màu
            fadeToBlackBy(leds2, NUM_LEDS_2, 80);
            leds2[chase % NUM_LEDS_2] = CHSV(hue, 255, 255);
            leds2[(chase + NUM_LEDS_2 / 3) % NUM_LEDS_2] = CHSV(hue + 85, 255, 255);
            leds2[(chase + (NUM_LEDS_2 * 2) / 3) % NUM_LEDS_2] = CHSV(hue + 170, 255, 255);
            break;

        case 8: // Lấp lánh
            fadeToBlackBy(leds2, NUM_LEDS_2, 35);
            if (random8() < 160) {
                leds2[random16(NUM_LEDS_2)] += CRGB::White;
            }
            break;
            
        default:
            fill_solid(leds2, NUM_LEDS_2, CRGB::Black);
            break;
    }
}
