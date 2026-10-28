#pragma once

// NTP Configuration
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 25200 // GMT+7 = 7 * 3600
#define DAYLIGHT_OFFSET_SEC 0

// LED Configuration
#define LED_PIN 3
#define BACKGROUND_LED_PIN 4
#define NUM_LEDS 70
#define NUM_BACKGROUND_LEDS 60
#define MAX_RUNTIME_LEDS 256
#define CLOCK_LED_COUNT 60 
#define EFFECT_START_INDEX 61
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
#define BACKGROUND_EFFECT_DURATION_DEFAULT 5000
#define OTA_PASSWORD_DEFAULT "1234567890"
#define MQTT_PORT_DEFAULT 1883
#define MQTT_HOST_DEFAULT "homeassistant.local"
#define MQTT_TOPIC_DEFAULT "clock-led"
