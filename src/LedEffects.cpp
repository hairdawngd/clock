#include "LedEffects.h"

void renderLedEffect(CRGB *leds, uint16_t count, int effectType,
                     unsigned long elapsed, unsigned long duration,
                     uint8_t &hue, uint16_t &chase) {
    if (count == 0) return;
    hue++;
    chase++;

    switch (effectType) {
        case 1:
            fill_rainbow(leds, count, hue, 255 / count + 1);
            break;
        case 2:
            fadeToBlackBy(leds, count, 20);
            if (random8() < 120) leds[random16(count)] += CHSV(hue, 255, 255);
            break;
        case 3:
            fill_solid(leds, count, CHSV(hue, 255, beatsin8(30, 0, 255)));
            break;
        case 4:
            for (uint16_t i = 0; i < count; i++) {
                if (random8() < 5) leds[i] = CHSV(random8(), 255, 255);
                else leds[i].nscale8(250);
            }
            break;
        case 5:
            if ((millis() / 250) % 2) fill_solid(leds, count, CHSV(hue, 255, 255));
            else fill_solid(leds, count, CRGB::Black);
            break;
        case 6:
            for (uint16_t i = 0; i < count; i++) {
                leds[i] = HeatColor(random8(80, 255));
                leds[i].fadeToBlackBy(random8(0, 120));
            }
            break;
        case 7:
            fadeToBlackBy(leds, count, 80);
            leds[chase % count] = CHSV(hue, 255, 255);
            leds[(chase + count / 3) % count] = CHSV(hue + 85, 255, 255);
            leds[(chase + (count * 2) / 3) % count] = CHSV(hue + 170, 255, 255);
            break;
        case 8:
            fadeToBlackBy(leds, count, 35);
            if (random8() < 160) leds[random16(count)] += CRGB::White;
            break;
        case 9: {
            unsigned long sweepMs = duration * 2 / 5;
            if (sweepMs == 0) sweepMs = 1;
            if (elapsed < sweepMs) {
                int head = (int)((elapsed * count) / sweepMs);
                if (head >= count) head = count - 1;
                for (uint16_t i = 0; i < count; i++) {
                    if (i <= head) leds[i] = CHSV((i * 255) / count, 255, 255);
                    else leds[i] = CRGB::Black;
                }
            } else {
                unsigned long fadeMs = duration > sweepMs ? duration - sweepMs : 1;
                unsigned long fadeTime = elapsed - sweepMs;
                if (fadeTime > fadeMs) fadeTime = fadeMs;
                uint8_t value = 255 - (uint8_t)((fadeTime * 255) / fadeMs);
                fill_solid(leds, count, CRGB(value, value, value));
            }
            break;
        }
        default:
            fill_solid(leds, count, CRGB::Black);
            break;
    }
}
