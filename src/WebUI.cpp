#include "WebUI.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Update.h>
#include "Settings.h"
#include "Config.h"
#include "TimeManager.h"
#include "LedDisplay.h"

extern TimeManager timeManager;
extern LedDisplay display;

AsyncWebServer server(80);

static bool parseEffectTime(const String &value, int &minutes) {
  if (value.length() != 5 || value[2] != ':' ||
      value[0] < '0' || value[0] > '9' ||
      value[1] < '0' || value[1] > '9' ||
      value[3] < '0' || value[3] > '9' ||
      value[4] < '0' || value[4] > '9') {
    return false;
  }

  int hour = (value[0] - '0') * 10 + value[1] - '0';
  int minute = (value[3] - '0') * 10 + value[4] - '0';
  if (hour >= 24 || minute >= 60) return false;

  minutes = hour * 60 + minute;
  return true;
}

static String formatEffectTime(int minutes) {
  char value[6];
  snprintf(value, sizeof(value), "%02d:%02d", minutes / 60, minutes % 60);
  return String(value);
}

static bool parseColorParam(AsyncWebServerRequest *request, const char *name, uint32_t &color) {
  if (!request->hasParam(name, true)) return true;
  String value = request->getParam(name, true)->value();
  if (value.startsWith("#")) value = value.substring(1);
  if (value.length() != 6) return false;
  color = strtoul(value.c_str(), nullptr, 16);
  return true;
}

void handleScan(AsyncWebServerRequest *request) {
  int n = WiFi.scanComplete();
  JsonDocument doc;
  
  if (n == WIFI_SCAN_RUNNING || n == WIFI_SCAN_FAILED || n < 0) {
      if (n == WIFI_SCAN_FAILED || n < 0) WiFi.scanNetworks(true, true);
      JsonArray array = doc.to<JsonArray>();
      String response;
      serializeJson(doc, response);
      request->send(200, "application/json", response);
      return;
  }

  JsonArray array = doc.to<JsonArray>();
  for(int i=0;i<n;i++){
      if (WiFi.SSID(i).length() > 0) {
          array.add(WiFi.SSID(i));
      }
  }
  
  String response;
  serializeJson(doc, response);
  
  WiFi.scanDelete();
  WiFi.scanNetworks(true, true);
  request->send(200, "application/json", response);
}

void handleWifiPage(AsyncWebServerRequest *request) {
  File f = SPIFFS.open("/wifi.html", "r");
  if (!f) {
    request->send(404, "text/html", "Không tìm thấy wifi.html");
    return;
  }
  String html = f.readString();
  f.close();
  request->send(200, "text/html", html);
}

void handleConnect(AsyncWebServerRequest *request) {
  String ssid, pwd;
  if (request->hasParam("ssid", true))
    ssid = request->getParam("ssid", true)->value();
  if (request->hasParam("password", true))
    pwd = request->getParam("password", true)->value();

  if (!ssid.length()) {
    Serial.println("[ERROR] Missing SSID. Cannot connect.");
    JsonDocument errDoc;
    errDoc["error"] = "No SSID.";
    String errResp;
    serializeJson(errDoc, errResp);
    request->send(400, "application/json", errResp);
    return;
  }

  Serial.printf("[WIFI] Received Connect Request -> SSID: %s; PWD: %s\n", ssid.c_str(), pwd.c_str());

  wifiSSID = ssid;
  wifiPass = pwd;
  saveSettings();

  Serial.println("[WIFI] Config saved. Restarting; boot will connect STA or fallback AP.");

  JsonDocument doc;
  doc["status"] = "ok";
  doc["message"] = "Đã lưu WiFi. ESP32 sẽ khởi động lại và kết nối WiFi.";
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);

  xTaskCreate(
      [](void *param) {
        vTaskDelay(1200 / portTICK_PERIOD_MS);
        ESP.restart();
        vTaskDelete(nullptr);
      },
      "restart_after_wifi_save",
      2048,
      nullptr,
      1,
      nullptr);
}

void handleRoot(AsyncWebServerRequest *request) {
    File f = SPIFFS.open("/setup.html", "r");
    if (!f) {
        request->send(404, "text/html", "Không tìm thấy setup.html");
        return;
    }
    String html = f.readString();
    f.close();
    request->send(200, "text/html", html);
}

void handleApiSettings(AsyncWebServerRequest *request) {
    char hexH[8], hexM[8], hexS[8], hexMark[8];
    snprintf(hexH, sizeof(hexH), "#%06lx", (unsigned long)(colorHour & 0xFFFFFF));
    snprintf(hexM, sizeof(hexM), "#%06lx", (unsigned long)(colorMinute & 0xFFFFFF));
    snprintf(hexS, sizeof(hexS), "#%06lx", (unsigned long)(colorSecond & 0xFFFFFF));
    snprintf(hexMark, sizeof(hexMark), "#%06lx", (unsigned long)(colorMarker & 0xFFFFFF));
    
    JsonDocument doc;
    doc["chour"] = hexH;
    doc["cmin"] = hexM;
    doc["csec"] = hexS;
    doc["cmark"] = hexMark;
    doc["minute_effect"] = minuteEffectDuration;
    doc["hour_effect"] = hourEffectDuration;
    doc["effect_start"] = formatEffectTime(effectWindowStartMinute);
    doc["effect_end"] = formatEffectTime(effectWindowEndMinute);
    doc["gmt_offset"] = gmtOffsetHours;
    
    doc["eff_minute"] = effectMinute2;
    doc["eff_hour"] = effectHour2;
    doc["brightness"] = brightness;
    doc["display_enabled"] = displayEnabled;
    doc["background_enabled"] = backgroundEnabled;
    snprintf(hexMark, sizeof(hexMark), "#%06lx", (unsigned long)(backgroundColor & 0xFFFFFF));
    doc["background_color"] = hexMark;
    doc["background_effect"] = backgroundEffect;
    doc["background_effect_duration"] = backgroundEffectDuration;
    doc["background_start"] = formatEffectTime(backgroundWindowStartMinute);
    doc["background_end"] = formatEffectTime(backgroundWindowEndMinute);
    doc["display_pin"] = displayLedPin;
    doc["display_count"] = displayLedCount;
    doc["background_pin"] = backgroundLedPin;
    doc["background_count"] = backgroundLedCount;
    doc["mqtt_enabled"] = mqttEnabled;
    doc["mqtt_discovery"] = mqttDiscovery;
    doc["mqtt_host"] = mqttHost;
    doc["mqtt_port"] = mqttPort;
    doc["mqtt_user"] = mqttUser;
    doc["mqtt_topic"] = mqttTopic;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void handleGetTime(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["time"] = timeManager.getTimeString();
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}
void handleSync(AsyncWebServerRequest *request) {
    timeManager.syncTime();
    JsonDocument doc;
    doc["status"] = "ok";
    doc["time"] = timeManager.getTimeString();
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}
void handleTestEffect(AsyncWebServerRequest *request) {
    if (request->hasParam("type")) {
        String type = request->getParam("type")->value();
        if (type == "minute") display.triggerEffect(STATE_MINUTE_EFFECT);
        else if (type == "hour") display.triggerEffect(STATE_HOUR_EFFECT);
    }
    JsonDocument doc;
    doc["status"] = "ok";
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void handleTestColor(AsyncWebServerRequest *request) {
    if (request->hasParam("chour", true) && request->hasParam("cmin", true) && request->hasParam("csec", true)) {
        String vH = request->getParam("chour", true)->value();
        String vM = request->getParam("cmin", true)->value();
        String vS = request->getParam("csec", true)->value();
        if (vH.startsWith("#")) vH = vH.substring(1);
        if (vM.startsWith("#")) vM = vM.substring(1);
        if (vS.startsWith("#")) vS = vS.substring(1);
        
        display.updateColorLive(
            strtoul(vH.c_str(), nullptr, 16),
            strtoul(vM.c_str(), nullptr, 16),
            strtoul(vS.c_str(), nullptr, 16)
        );
    }
    if (request->hasParam("cmark", true)) {
        String val = request->getParam("cmark", true)->value();
        if (val.startsWith("#")) val = val.substring(1);
        colorMarker = strtoul(val.c_str(), nullptr, 16);
    }
    request->send(200, "text/plain", "ok");
}
void handleSave(AsyncWebServerRequest *request) {
    bool ledHardwareChanged = false;
    int configuredStart = effectWindowStartMinute;
    int configuredEnd = effectWindowEndMinute;
    if ((request->hasParam("effect_start", true) &&
         !parseEffectTime(request->getParam("effect_start", true)->value(), configuredStart)) ||
        (request->hasParam("effect_end", true) &&
         !parseEffectTime(request->getParam("effect_end", true)->value(), configuredEnd))) {
        JsonDocument doc;
        doc["error"] = "Khung giờ hiệu ứng phải đúng định dạng HH:mm (00:00 đến 23:59).";
        String response;
        serializeJson(doc, response);
        request->send(400, "application/json", response);
        return;
    }

    effectWindowStartMinute = configuredStart;
    effectWindowEndMinute = configuredEnd;

    int configuredBackgroundStart = backgroundWindowStartMinute;
    int configuredBackgroundEnd = backgroundWindowEndMinute;
    if ((request->hasParam("background_start", true) &&
         !parseEffectTime(request->getParam("background_start", true)->value(), configuredBackgroundStart)) ||
        (request->hasParam("background_end", true) &&
         !parseEffectTime(request->getParam("background_end", true)->value(), configuredBackgroundEnd))) {
        request->send(400, "application/json",
                      "{\"error\":\"Khung giờ LED nền không hợp lệ.\"}");
        return;
    }
    backgroundWindowStartMinute = configuredBackgroundStart;
    backgroundWindowEndMinute = configuredBackgroundEnd;

    if (request->hasParam("chour", true)) {
        String val = request->getParam("chour", true)->value();
        if (val.startsWith("#")) val = val.substring(1);
        colorHour = strtoul(val.c_str(), nullptr, 16);
    }
    if (request->hasParam("cmin", true)) {
        String val = request->getParam("cmin", true)->value();
        if (val.startsWith("#")) val = val.substring(1);
        colorMinute = strtoul(val.c_str(), nullptr, 16);
    }
    if (request->hasParam("csec", true)) {
        String val = request->getParam("csec", true)->value();
        if (val.startsWith("#")) val = val.substring(1);
        colorSecond = strtoul(val.c_str(), nullptr, 16);
    }
    if (request->hasParam("cmark", true)) {
        String val = request->getParam("cmark", true)->value();
        if (val.startsWith("#")) val = val.substring(1);
        colorMarker = strtoul(val.c_str(), nullptr, 16);
    }
    if (!parseColorParam(request, "background_color", backgroundColor)) {
        request->send(400, "application/json",
                      "{\"error\":\"Màu LED nền không hợp lệ.\"}");
        return;
    }
    if (request->hasParam("minute_effect", true)) minuteEffectDuration = request->getParam("minute_effect", true)->value().toInt();
    if (request->hasParam("hour_effect", true)) hourEffectDuration = request->getParam("hour_effect", true)->value().toInt();
    if (request->hasParam("gmt_offset", true)) gmtOffsetHours = request->getParam("gmt_offset", true)->value().toInt();
    
    if (request->hasParam("eff_minute", true)) effectMinute2 = request->getParam("eff_minute", true)->value().toInt();
    if (request->hasParam("eff_hour", true)) effectHour2 = request->getParam("eff_hour", true)->value().toInt();
    if (request->hasParam("brightness", true)) brightness = constrain(request->getParam("brightness", true)->value().toInt(), 0, 255);
    if (request->hasParam("display_enabled", true)) displayEnabled = request->getParam("display_enabled", true)->value() == "1";
    if (request->hasParam("background_enabled", true)) backgroundEnabled = request->getParam("background_enabled", true)->value() == "1";
    if (request->hasParam("background_effect", true)) backgroundEffect = constrain(request->getParam("background_effect", true)->value().toInt(), 0, 9);
    if (request->hasParam("background_effect_duration", true)) backgroundEffectDuration = constrain(request->getParam("background_effect_duration", true)->value().toInt(), 1000, 60000);
    if (request->hasParam("ota_password", true) && request->getParam("ota_password", true)->value().length() >= 8) {
        otaPassword = request->getParam("ota_password", true)->value();
    }
    if (request->hasParam("display_pin", true)) {
        int value = request->getParam("display_pin", true)->value().toInt();
        ledHardwareChanged |= value != displayLedPin;
        displayLedPin = value;
    }
    if (request->hasParam("display_count", true)) {
        int value = constrain(request->getParam("display_count", true)->value().toInt(), 1, MAX_RUNTIME_LEDS);
        ledHardwareChanged |= value != displayLedCount;
        displayLedCount = value;
    }
    if (request->hasParam("background_pin", true)) {
        int value = request->getParam("background_pin", true)->value().toInt();
        ledHardwareChanged |= value != backgroundLedPin;
        backgroundLedPin = value;
    }
    if (request->hasParam("background_count", true)) {
        int value = constrain(request->getParam("background_count", true)->value().toInt(), 1, MAX_RUNTIME_LEDS);
        ledHardwareChanged |= value != backgroundLedCount;
        backgroundLedCount = value;
    }
    if (request->hasParam("mqtt_enabled", true)) mqttEnabled = request->getParam("mqtt_enabled", true)->value() == "1";
    if (request->hasParam("mqtt_discovery", true)) mqttDiscovery = request->getParam("mqtt_discovery", true)->value() == "1";
    if (request->hasParam("mqtt_host", true)) mqttHost = request->getParam("mqtt_host", true)->value();
    if (request->hasParam("mqtt_port", true)) mqttPort = constrain(request->getParam("mqtt_port", true)->value().toInt(), 1, 65535);
    if (request->hasParam("mqtt_user", true)) mqttUser = request->getParam("mqtt_user", true)->value();
    if (request->hasParam("mqtt_pass", true) && request->getParam("mqtt_pass", true)->value().length()) mqttPass = request->getParam("mqtt_pass", true)->value();
    if (request->hasParam("mqtt_topic", true)) mqttTopic = request->getParam("mqtt_topic", true)->value();
    
    saveSettings();
    display.updateColorLive(colorHour, colorMinute, colorSecond);
    timeManager.syncTime();
    
    JsonDocument doc;
    doc["message"] = "Đã lưu cài đặt.";
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
    if (ledHardwareChanged) {
        xTaskCreate([](void *) {
            vTaskDelay(1200 / portTICK_PERIOD_MS);
            ESP.restart();
            vTaskDelete(nullptr);
        }, "restart_led_config", 2048, nullptr, 1, nullptr);
    }
}

void handleOtaUpload(AsyncWebServerRequest *request, String filename, size_t index,
                     uint8_t *data, size_t len, bool final) {
    if (!request->authenticate("admin", otaPassword.c_str())) return;
    if (index == 0) {
        Serial.printf("[OTA] Uploading %s\n", filename.c_str());
        filename.toLowerCase();
        int command = filename.endsWith("spiffs.bin") ? U_SPIFFS : U_FLASH;
        if (!filename.endsWith("firmware.bin") && command != U_SPIFFS) {
            Serial.println("[OTA] Rejected file. Use firmware.bin or spiffs.bin.");
            return;
        }
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, command)) Update.printError(Serial);
    }
    if (Update.write(data, len) != len) Update.printError(Serial);
    if (final) {
        if (!Update.end(true)) Update.printError(Serial);
        else Serial.printf("[OTA] Upload complete: %u bytes\n", index + len);
    }
}
void handleNotFound(AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["error"] = "Not Found";
    String response;
    serializeJson(doc, response);
    request->send(404, "application/json", response);
}

void setupWebUI() {
    server.serveStatic("/style.css", SPIFFS, "/style.css");
    server.serveStatic("/wifi.js", SPIFFS, "/wifi.js");
    server.serveStatic("/app.js", SPIFFS, "/app.js");
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/settings", HTTP_GET, handleApiSettings);
    server.on("/api/save", HTTP_POST, handleSave);
    server.on("/api/time", HTTP_GET, handleGetTime);
    server.on("/sync", HTTP_GET, handleSync);
    server.on("/test_effect", HTTP_GET, handleTestEffect);
    server.on("/api/test_color", HTTP_POST, handleTestColor);
    server.on("/connectwifi", HTTP_GET, handleWifiPage);
    server.on("/scan", HTTP_GET, handleScan);
    server.on("/connect", HTTP_POST, handleConnect);
    auto otaResponse = [](AsyncWebServerRequest *request) {
            if (!request->authenticate("admin", otaPassword.c_str())) {
                request->requestAuthentication();
                return;
            }
            if (Update.hasError()) request->send(500, "text/plain", "OTA thất bại");
            else request->send(200, "text/plain", "OTA thành công, thiết bị đang khởi động lại.");
            delay(100);
            ESP.restart();
        };
    server.on("/update", HTTP_POST,
        otaResponse,
        [](AsyncWebServerRequest *request, String filename, size_t index,
           uint8_t *data, size_t len, bool final) {
            handleOtaUpload(request, filename, index, data, len, final);
        });
    server.onNotFound(handleNotFound);
    server.begin();
}