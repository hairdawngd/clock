#include "MqttManager.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Settings.h"

static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);
static unsigned long lastAttempt = 0;

static String colorHex(uint32_t color) {
    char value[8];
    snprintf(value, sizeof(value), "#%06lx", (unsigned long)(color & 0xFFFFFF));
    return String(value);
}

static void addDevice(JsonDocument &doc) {
    doc["device"]["identifiers"][0] = "clock_led";
    doc["device"]["name"] = "LED Clock";
    doc["device"]["manufacturer"] = "forme";
    doc["device"]["model"] = "ESP32 LED Clock";
}

static void publishSensorDiscovery(const char *id, const char *name,
                                   const char *templateValue, const char *unit = nullptr) {
    JsonDocument doc;
    doc["name"] = name;
    doc["uniq_id"] = String("clock_led_") + id;
    doc["stat_t"] = mqttTopic + "/state";
    doc["val_tpl"] = templateValue;
    if (unit) doc["unit_of_meas"] = unit;
    addDevice(doc);
    String topic = String("homeassistant/sensor/clock_led_") + id + "/config";
    String payload;
    serializeJson(doc, payload);
    mqtt.publish(topic.c_str(), payload.c_str(), true);
}

static void publishDiscovery() {
    if (!mqttDiscovery) return;
    JsonDocument doc;
    doc["name"] = "LED Clock";
    doc["uniq_id"] = "clock_led";
    doc["stat_t"] = mqttTopic + "/state";
    doc["cmd_t"] = mqttTopic + "/set";
    doc["schema"] = "json";
    addDevice(doc);
    String payload;
    serializeJson(doc, payload);
    mqtt.publish("homeassistant/light/clock_led/config", payload.c_str(), true);
    publishSensorDiscovery("color_hour", "LED Clock Hour Color", "{{ value_json.color_hour }}");
    publishSensorDiscovery("color_minute", "LED Clock Minute Color", "{{ value_json.color_minute }}");
    publishSensorDiscovery("color_second", "LED Clock Second Color", "{{ value_json.color_second }}");
    publishSensorDiscovery("color_marker", "LED Clock Marker Color", "{{ value_json.color_marker }}");
    publishSensorDiscovery("marker_0", "LED Clock Marker 0", "{{ value_json.marker_0 }}");
    publishSensorDiscovery("marker_15", "LED Clock Marker 15", "{{ value_json.marker_15 }}");
    publishSensorDiscovery("marker_30", "LED Clock Marker 30", "{{ value_json.marker_30 }}");
    publishSensorDiscovery("marker_45", "LED Clock Marker 45", "{{ value_json.marker_45 }}");
    publishSensorDiscovery("minute_effect", "LED Clock Minute Effect", "{{ value_json.minute_effect }}");
    publishSensorDiscovery("hour_effect", "LED Clock Hour Effect", "{{ value_json.hour_effect }}");
    publishSensorDiscovery("background_color", "LED Clock Background Color", "{{ value_json.background_color }}");
    publishSensorDiscovery("background_effect", "LED Clock Background Effect", "{{ value_json.background_effect }}");
}

static void publishState() {
    JsonDocument doc;
    doc["display"] = displayEnabled;
    doc["background"] = backgroundEnabled;
    doc["brightness"] = brightness;
    doc["color_hour"] = colorHex(colorHour);
    doc["color_minute"] = colorHex(colorMinute);
    doc["color_second"] = colorHex(colorSecond);
    doc["color_marker"] = colorHex(colorMarker);
    doc["marker_0"] = colorHex(colorMarker);
    doc["marker_15"] = colorHex(colorMarker);
    doc["marker_30"] = colorHex(colorMarker);
    doc["marker_45"] = colorHex(colorMarker);
    doc["minute_effect"] = effectMinute2;
    doc["hour_effect"] = effectHour2;
    doc["background_color"] = colorHex(backgroundColor);
    doc["background_effect"] = backgroundEffect;
    doc["minute_effect_duration"] = minuteEffectDuration;
    doc["hour_effect_duration"] = hourEffectDuration;
    doc["background_effect_duration"] = backgroundEffectDuration;
    doc["background_start"] = backgroundWindowStartMinute;
    doc["background_end"] = backgroundWindowEndMinute;
    String payload;
    serializeJson(doc, payload);
    mqtt.publish((mqttTopic + "/state").c_str(), payload.c_str(), true);
}

static void onMessage(char *, uint8_t *data, unsigned int length) {
    String value;
    for (unsigned int i = 0; i < length; i++) value += (char)data[i];
    JsonDocument doc;
    if (deserializeJson(doc, value)) return;
    if (doc["display"].is<bool>()) displayEnabled = doc["display"];
    if (doc["background"].is<bool>()) backgroundEnabled = doc["background"];
    if (doc["brightness"].is<int>()) brightness = constrain((int)doc["brightness"], 0, 255);
    if (doc["color_hour"].is<const char *>()) colorHour = strtoul(doc["color_hour"].as<const char *>(), nullptr, 16);
    if (doc["color_minute"].is<const char *>()) colorMinute = strtoul(doc["color_minute"].as<const char *>(), nullptr, 16);
    if (doc["color_second"].is<const char *>()) colorSecond = strtoul(doc["color_second"].as<const char *>(), nullptr, 16);
    if (doc["color_marker"].is<const char *>()) colorMarker = strtoul(doc["color_marker"].as<const char *>(), nullptr, 16);
    if (doc["marker_0"].is<const char *>()) colorMarker = strtoul(doc["marker_0"].as<const char *>(), nullptr, 16);
    if (doc["marker_15"].is<const char *>()) colorMarker = strtoul(doc["marker_15"].as<const char *>(), nullptr, 16);
    if (doc["marker_30"].is<const char *>()) colorMarker = strtoul(doc["marker_30"].as<const char *>(), nullptr, 16);
    if (doc["marker_45"].is<const char *>()) colorMarker = strtoul(doc["marker_45"].as<const char *>(), nullptr, 16);
    if (doc["minute_effect"].is<int>()) effectMinute2 = constrain((int)doc["minute_effect"], 0, 9);
    if (doc["hour_effect"].is<int>()) effectHour2 = constrain((int)doc["hour_effect"], 0, 9);
    if (doc["background_color"].is<const char *>()) backgroundColor = strtoul(doc["background_color"].as<const char *>(), nullptr, 16);
    if (doc["background_effect"].is<int>()) backgroundEffect = constrain((int)doc["background_effect"], 0, 9);
    if (doc["minute_effect_duration"].is<int>()) minuteEffectDuration = constrain((int)doc["minute_effect_duration"], 1000, 60000);
    if (doc["hour_effect_duration"].is<int>()) hourEffectDuration = constrain((int)doc["hour_effect_duration"], 1000, 60000);
    if (doc["background_effect_duration"].is<int>()) backgroundEffectDuration = constrain((int)doc["background_effect_duration"], 1000, 60000);
    saveSettings();
    publishState();
}

void mqttInit() {
    mqtt.setCallback(onMessage);
    mqtt.setBufferSize(1024);
}

void mqttUpdate() {
    if (!mqttEnabled || WiFi.status() != WL_CONNECTED || !mqttHost.length()) return;
    if (!mqtt.connected()) {
        if (millis() - lastAttempt < 5000) return;
        lastAttempt = millis();
        mqtt.setServer(mqttHost.c_str(), mqttPort);
        String clientId = "clock-led-" + String((uint32_t)ESP.getEfuseMac(), HEX);
        bool connected = mqttUser.length()
            ? mqtt.connect(clientId.c_str(), mqttUser.c_str(), mqttPass.c_str())
            : mqtt.connect(clientId.c_str());
        if (connected) {
            mqtt.subscribe((mqttTopic + "/set").c_str());
            publishDiscovery();
            publishState();
        }
        return;
    }
    mqtt.loop();
}
