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
    preserveClockEdge(hour, minute, second);
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


void LedDisplay::drawMinuteEffect() {
    // Vùng effect bắt đầu từ led 60 trở đi
    static uint8_t hue = 0;
    hue += 5;
    if(NUM_LEDS <= EFFECT_START_INDEX) return; // không đủ led cho effect
    fadeToBlackBy(leds + EFFECT_START_INDEX, NUM_LEDS-EFFECT_START_INDEX, 20);

    if (random8() < 120) {
        int pos = random16(NUM_LEDS-EFFECT_START_INDEX) + EFFECT_START_INDEX;
        leds[pos] += CHSV(random8(), 255, 255);
    }
}

void LedDisplay::drawHourEffect() {
    static uint8_t startIndex = 0;
    startIndex += 2;
    if(NUM_LEDS <= EFFECT_START_INDEX) return;
    fadeToBlackBy(leds+EFFECT_START_INDEX, NUM_LEDS-EFFECT_START_INDEX, 20);

    CRGB rainbow[NUM_LEDS-EFFECT_START_INDEX];
    fill_rainbow(rainbow, NUM_LEDS-EFFECT_START_INDEX, startIndex, (NUM_LEDS-EFFECT_START_INDEX) ? (255/(NUM_LEDS-EFFECT_START_INDEX)) : 255);

    for(int i = 0; i < NUM_LEDS-EFFECT_START_INDEX; i++) {
        if (random8() < 50) {
           leds[i+EFFECT_START_INDEX] = rainbow[i];
        }
    }
}

void LedDisplay::drawEffect2(int effectType) {
    static uint8_t hue = 0;
    static uint16_t chase = 0;
    hue++;
    chase++;
    if(NUM_LEDS <= EFFECT_START_INDEX) return;
    CRGB* eleds = leds+EFFECT_START_INDEX;
    int elen = NUM_LEDS-EFFECT_START_INDEX;
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
