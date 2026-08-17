#pragma once

// NTP Configuration
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 25200 // GMT+7 = 7 * 3600
#define DAYLIGHT_OFFSET_SEC 0

// LED Configuration
#define LED_PIN 2
#define NUM_LEDS 70 // tổng số led vật lý (1-60: clock, 61+: effect, thay theo dải thật)
#define CLOCK_LED_COUNT 60 // số led làm mặt đồng hồ, chỉnh theo yêu cầu
#define BRIGHTNESS 255
#define LED_TYPE WS2812
#define COLOR_ORDER GRB

// Colors (CRGB)
#define COLOR_HOUR_DEFAULT      0x0000FF // Blue
#define COLOR_MINUTE_DEFAULT    0x00FF00 // Green
#define COLOR_SECOND_DEFAULT    0xFF0000   // Red
#define COLOR_BG 0x050505     // Dim white for background / markings

#define MINUTE_EFFECT_DURATION_DEFAULT 5000
#define HOUR_EFFECT_DURATION_DEFAULT 5000
