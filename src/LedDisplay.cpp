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

static int mapClockValueToLed(int value) {
    value %= CLOCK_LED_COUNT;
    if (value < 0) value += CLOCK_LED_COUNT;
    return value;
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

    nscale8_video(leds, NUM_LEDS, constrain(brightness, 0, 255));
    FastLED.show();
}

void LedDisplay::drawNormalTime(int h, int m, int s) {
    m = mapClockValueToLed(m);
    s = mapClockValueToLed(s);

    for (int i = 0; i < CLOCK_LED_COUNT && i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }

    // Fixed clock markers: 12, 3, 6, 9. Draw first so hands can overlap them.
    const int markers[] = {0, 15, 30, 45};
    for (int marker : markers) {
        if (marker < NUM_LEDS) leds[marker] = colorMarker;
    }

    // led[0] is physical LED 1 and clock position 12.
    // Vẽ giờ sau cùng để kim giờ không bị giây/phút đè (giây quét mọi vị trí mỗi phút).
    leds[s] = colorSecond;
    leds[m] = colorMinute;
    leds[mapHourToLed(h)] = colorHour;
}


void LedDisplay::drawMinuteEffect() {
    // Vùng effect bắt đầu từ led 60 trở đi
    static uint8_t hue = 0;
    hue += 5;
    if(NUM_LEDS <= CLOCK_LED_COUNT) return; // không đủ led cho effect
    fadeToBlackBy(leds + CLOCK_LED_COUNT, NUM_LEDS-CLOCK_LED_COUNT, 20);

    if (random8() < 120) {
        int pos = random16(NUM_LEDS-CLOCK_LED_COUNT) + CLOCK_LED_COUNT;
        leds[pos] += CHSV(random8(), 255, 255);
    }
}

void LedDisplay::drawHourEffect() {
    static uint8_t startIndex = 0;
    startIndex += 2;
    if(NUM_LEDS <= CLOCK_LED_COUNT) return;
    fadeToBlackBy(leds+CLOCK_LED_COUNT, NUM_LEDS-CLOCK_LED_COUNT, 20);

    CRGB rainbow[NUM_LEDS-CLOCK_LED_COUNT];
    fill_rainbow(rainbow, NUM_LEDS-CLOCK_LED_COUNT, startIndex, (NUM_LEDS-CLOCK_LED_COUNT) ? (255/(NUM_LEDS-CLOCK_LED_COUNT)) : 255);

    for(int i = 0; i < NUM_LEDS-CLOCK_LED_COUNT; i++) {
        if (random8() < 50) {
           leds[i+CLOCK_LED_COUNT] = rainbow[i];
        }
    }
}

void LedDisplay::drawEffect2(int effectType) {
    static uint8_t hue = 0;
    static uint16_t chase = 0;
    hue++;
    chase++;
    if(NUM_LEDS <= CLOCK_LED_COUNT) return;
    CRGB* eleds = leds+CLOCK_LED_COUNT;
    int elen = NUM_LEDS-CLOCK_LED_COUNT;
    switch (effectType) {
        case 0: // Tắt
            fill_solid(eleds, elen, CRGB::Black);
            break;
        case 1: // Cầu vồng
            fill_rainbow(eleds, elen, hue, elen ? (255/elen) : 255);
            break;
        case 2: // Sao băng
            fadeToBlackBy(eleds, elen, 20);
            if (random8() < 120) {
                eleds[random16(elen)] += CHSV(hue, 255, 255);
            }
            break;
        case 3: // Nhịp thở
        {
            uint8_t b = beatsin8(30, 0, 255);
            fill_solid(eleds, elen, CHSV(hue, 255, b));
            break;
        }
        case 4: // Đổi màu ngẫu nhiên (chớp ngẫu nhiên)
            for(int i=0; i<elen; i++) {
                if(random8() < 5) eleds[i] = CHSV(random8(), 255, 255);
                else eleds[i].nscale8(250);
            }
            break;
        case 5: // Chớp nháy
            if ((millis() / 250) % 2) {
                fill_solid(eleds, elen, CHSV(hue, 255, 255));
            } else {
                fill_solid(eleds, elen, CRGB::Black);
            }
            break;
        case 6: // Lửa
            for (int i = 0; i < elen; i++) {
                eleds[i] = HeatColor(random8(80, 255));
                eleds[i].fadeToBlackBy(random8(0, 120));
            }
            break;
        case 7: // Đuổi màu
            fadeToBlackBy(eleds, elen, 80);
            eleds[chase % elen] = CHSV(hue, 255, 255);
            eleds[(chase + elen / 3) % elen] = CHSV(hue + 85, 255, 255);
            eleds[(chase + (elen * 2) / 3) % elen] = CHSV(hue + 170, 255, 255);
            break;
        case 8: // Lấp lánh
            fadeToBlackBy(eleds, elen, 35);
            if (random8() < 160) {
                eleds[random16(elen)] += CRGB::White;
            }
            break;
        default:
            fill_solid(eleds, elen, CRGB::Black);
            break;
    }
}
