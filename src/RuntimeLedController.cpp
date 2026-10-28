#include "RuntimeLedController.h"

template <uint8_t Pin>
static CLEDController *addController(CRGB *leds, int count) {
    CLEDController &controller = FastLED.addLeds<WS2812, Pin, GRB>(leds, count);
    controller.setCorrection(TypicalLEDStrip);
    return &controller;
}

CLEDController *addRuntimeLedController(int pin, CRGB *leds, int count) {
    switch (pin) {
        case 0: return addController<0>(leds, count);
        case 1: return addController<1>(leds, count);
        case 2: return addController<2>(leds, count);
        case 3: return addController<3>(leds, count);
        case 4: return addController<4>(leds, count);
        case 5: return addController<5>(leds, count);
        case 6: return addController<6>(leds, count);
        case 7: return addController<7>(leds, count);
        case 8: return addController<8>(leds, count);
        case 9: return addController<9>(leds, count);
        case 10: return addController<10>(leds, count);
        default: return nullptr;
    }
}
