#include "Settings.h"
#include <Preferences.h>
#include "Config.h"

uint32_t colorHour = COLOR_HOUR_DEFAULT;
uint32_t colorMinute = COLOR_MINUTE_DEFAULT;
uint32_t colorSecond = COLOR_SECOND_DEFAULT;
uint32_t colorMarker = 0x000000;
uint32_t minuteEffectDuration = MINUTE_EFFECT_DURATION_DEFAULT;
uint32_t hourEffectDuration = HOUR_EFFECT_DURATION_DEFAULT;
int gmtOffsetHours = 7;
int effectWindowStartMinute = 0;
int effectWindowEndMinute = 23 * 60 + 59;

int effectMinute2 = 1;
int effectHour2 = 2;
int brightness = BRIGHTNESS;
bool displayEnabled = true;
bool backgroundEnabled = true;
uint32_t backgroundColor = COLOR_BG;
int backgroundEffect = 0;
uint32_t backgroundEffectDuration = BACKGROUND_EFFECT_DURATION_DEFAULT;
int backgroundWindowStartMinute = 0;
int backgroundWindowEndMinute = 23 * 60 + 59;
String otaPassword = OTA_PASSWORD_DEFAULT;
int displayLedPin = LED_PIN;
int displayLedCount = NUM_LEDS;
int backgroundLedPin = BACKGROUND_LED_PIN;
int backgroundLedCount = NUM_BACKGROUND_LEDS;
String mqttHost = MQTT_HOST_DEFAULT;
int mqttPort = MQTT_PORT_DEFAULT;
String mqttUser = "";
String mqttPass = "";
String mqttTopic = MQTT_TOPIC_DEFAULT;
bool mqttEnabled = false;
bool mqttDiscovery = true;

String wifiSSID = "";
String wifiPass = "";

Preferences prefs;

void loadSettings() {
    prefs.begin("clock", true);
    colorHour = prefs.getUInt("c_hour", COLOR_HOUR_DEFAULT);
    colorMinute = prefs.getUInt("c_min", COLOR_MINUTE_DEFAULT);
    colorSecond = prefs.getUInt("c_sec", COLOR_SECOND_DEFAULT);
    uint32_t cm = prefs.getUInt("c_mark", 0x000000);
    if (cm == 0x202020) cm = 0x000000; // default cũ sáng trắng 0x202020, khóa NVS đã lưu
    colorMarker = cm;
    minuteEffectDuration = prefs.getUInt("e_min", MINUTE_EFFECT_DURATION_DEFAULT);
    hourEffectDuration = prefs.getUInt("e_hour", HOUR_EFFECT_DURATION_DEFAULT);
    gmtOffsetHours = prefs.getInt("gmt", 7);
    effectWindowStartMinute = prefs.getInt("fx_start", 0);
    effectWindowEndMinute = prefs.getInt("fx_end", 23 * 60 + 59);
    if (effectWindowStartMinute < 0 || effectWindowStartMinute >= 24 * 60) {
        effectWindowStartMinute = 0;
    }
    if (effectWindowEndMinute < 0 || effectWindowEndMinute >= 24 * 60) {
        effectWindowEndMinute = 23 * 60 + 59;
    }
    effectMinute2 = prefs.getInt("eff_minute", 1);
    effectHour2 = prefs.getInt("eff_hour", 2);
    brightness = prefs.getInt("brightness", BRIGHTNESS);
    displayEnabled = prefs.getBool("display_on", true);
    backgroundEnabled = prefs.getBool("bg_on", true);
    backgroundColor = prefs.getUInt("bg_color", COLOR_BG);
    backgroundEffect = prefs.getInt("bg_effect", 0);
    backgroundEffectDuration = prefs.getUInt("bg_dur", BACKGROUND_EFFECT_DURATION_DEFAULT);
    backgroundWindowStartMinute = prefs.getInt("bg_start", 0);
    backgroundWindowEndMinute = prefs.getInt("bg_end", 23 * 60 + 59);
    if (backgroundWindowStartMinute < 0 || backgroundWindowStartMinute >= 24 * 60) {
        backgroundWindowStartMinute = 0;
    }
    if (backgroundWindowEndMinute < 0 || backgroundWindowEndMinute >= 24 * 60) {
        backgroundWindowEndMinute = 23 * 60 + 59;
    }
    otaPassword = prefs.getString("ota_pass", OTA_PASSWORD_DEFAULT);
    if (!otaPassword.length()) otaPassword = OTA_PASSWORD_DEFAULT;
    displayLedPin = prefs.getInt("d_pin", LED_PIN);
    displayLedCount = constrain(prefs.getInt("d_count", NUM_LEDS), 1, MAX_RUNTIME_LEDS);
    backgroundLedPin = prefs.getInt("b_pin", BACKGROUND_LED_PIN);
    backgroundLedCount = constrain(prefs.getInt("b_count", NUM_BACKGROUND_LEDS), 1, MAX_RUNTIME_LEDS);
    mqttHost = prefs.getString("mq_host", MQTT_HOST_DEFAULT);
    mqttPort = constrain(prefs.getInt("mq_port", MQTT_PORT_DEFAULT), 1, 65535);
    mqttUser = prefs.getString("mq_user", "");
    mqttPass = prefs.getString("mq_pass", "");
    mqttTopic = prefs.getString("mq_topic", MQTT_TOPIC_DEFAULT);
    mqttEnabled = prefs.getBool("mq_on", false);
    mqttDiscovery = prefs.getBool("mq_disc", true);
    wifiSSID = prefs.getString("w_ssid", "");
    wifiPass = prefs.getString("w_pass", "");
    prefs.end();
}

void saveSettings() {
    prefs.begin("clock", false);
    prefs.putUInt("c_hour", colorHour);
    prefs.putUInt("c_min", colorMinute);
    prefs.putUInt("c_sec", colorSecond);
    prefs.putUInt("c_mark", colorMarker);
    prefs.putUInt("e_min", minuteEffectDuration);
    prefs.putUInt("e_hour", hourEffectDuration);
    prefs.putInt("gmt", gmtOffsetHours);
    prefs.putInt("fx_start", effectWindowStartMinute);
    prefs.putInt("fx_end", effectWindowEndMinute);
    prefs.putInt("eff_minute", effectMinute2);
    prefs.putInt("eff_hour", effectHour2);
    prefs.putInt("brightness", brightness);
    prefs.putBool("display_on", displayEnabled);
    prefs.putBool("bg_on", backgroundEnabled);
    prefs.putUInt("bg_color", backgroundColor);
    prefs.putInt("bg_effect", backgroundEffect);
    prefs.putUInt("bg_dur", backgroundEffectDuration);
    prefs.putInt("bg_start", backgroundWindowStartMinute);
    prefs.putInt("bg_end", backgroundWindowEndMinute);
    prefs.putString("ota_pass", otaPassword);
    prefs.putInt("d_pin", displayLedPin);
    prefs.putInt("d_count", displayLedCount);
    prefs.putInt("b_pin", backgroundLedPin);
    prefs.putInt("b_count", backgroundLedCount);
    prefs.putString("mq_host", mqttHost);
    prefs.putInt("mq_port", mqttPort);
    prefs.putString("mq_user", mqttUser);
    prefs.putString("mq_pass", mqttPass);
    prefs.putString("mq_topic", mqttTopic);
    prefs.putBool("mq_on", mqttEnabled);
    prefs.putBool("mq_disc", mqttDiscovery);
    prefs.putString("w_ssid", wifiSSID);
    prefs.putString("w_pass", wifiPass);
    prefs.end();
}

bool isBackgroundWindowActive(int hour, int minute) {
    if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) return false;
    int currentMinute = hour * 60 + minute;
    if (backgroundWindowStartMinute <= backgroundWindowEndMinute) {
        return currentMinute >= backgroundWindowStartMinute &&
               currentMinute <= backgroundWindowEndMinute;
    }
    return currentMinute >= backgroundWindowStartMinute ||
           currentMinute <= backgroundWindowEndMinute;
}

bool isEffectWindowActive(int hour, int minute) {
    if (hour < 0 || hour >= 24 || minute < 0 || minute >= 60) return false;

    int currentMinute = hour * 60 + minute;
    if (effectWindowStartMinute <= effectWindowEndMinute) {
        return currentMinute >= effectWindowStartMinute &&
               currentMinute <= effectWindowEndMinute;
    }

    return currentMinute >= effectWindowStartMinute ||
           currentMinute <= effectWindowEndMinute;
}
