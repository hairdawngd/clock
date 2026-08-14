#include "WebUI.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Settings.h"
#include "Config.h"
#include "TimeManager.h"
#include "LedDisplay.h"

extern TimeManager timeManager;
extern LedDisplay display;

AsyncWebServer server(80);

const char* htmlTemplate = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>LED Clock Setup</title>
  <style>
    body { font-family: sans-serif; margin: 2rem; background: #222; color: #eee; }
    .wifi-btn { display:block; background: linear-gradient(90deg,#40c9ff,#0081eb 80%); color: #fff; text-decoration: none; text-align: center; padding: 12px; border-radius: 6px; margin:12px 0 18px 0; font-weight:500; font-size:1.06em; letter-spacing:.04em; border:2px solid #3af; transition:.2s; }
    .wifi-btn:hover { background:#0081eb; border-color:#50d9ff; color:#fff; box-shadow:0 0 16px #0081ebb0; }
    button { padding: 12px; background: #29c2ff; color: white; border: none; border-radius: 8px; cursor: pointer; width: 100%; margin-top: 10px; font-size:1em; }
    .card { background: #333; padding: 20px; border-radius: 8px; margin: auto; }
    .time { font-size: 2rem; text-align: center; margin: 10px 0; font-family: monospace; }
    label, input { display: block; margin: 8px 0; }
  </style>
</head>
<body>
  <div class="card">
    <h2>LED Clock Setup</h2>
    <svg id="ledCircle" viewBox="0 0 300 300" width="100%" style="max-width:300px;display:block;margin:0 auto;"></svg>
    <div class="time" id="clock">--:--:--</div>
    <button onclick="syncTime()">Sync Time Now</button>
    <div style="display: flex; gap: 10px;">
      <button onclick="testEffect('minute')">Test Min Effect</button>
      <button onclick="testEffect('hour')">Test Hour Effect</button>
    </div>
    <a href="/connectwifi" class="wifi-btn">Connect to WiFi</a>
    <form action="/save" method="POST">
      <label>Hour Color (Hex):</label>
      <input type="color" name="chour" value="#%CHOUR%">
      <label>Minute Color (Hex):</label>
      <input type="color" name="cmin" value="#%CMIN%">
      <label>Second Color (Hex):</label>
      <input type="color" name="csec" value="#%CSEC%">
      <label>Minute Effect Duration (ms):</label>
      <input type="number" name="minute_effect" min="1000" max="10000" step="1000" value="%MINUTE_EFFECT%">
      <label>Hour Effect Duration (ms):</label>
      <input type="number" name="hour_effect" min="1000" max="10000" step="1000" value="%HOUR_EFFECT%">
      <label>GMT Offset (Hours):</label>
      <input type="number" name="gmt_offset" min="-12" max="14" step="1" value="%GMT_OFFSET%">
      <button type="submit">Save Changes</button>
    </form>
    <div style="margin-top:12px;font-size:0.9em;color:#ccc;">
      WiFi setup handled by WiFiManager portal.<br/>
      Access <code>192.168.4.1</code> to reconfigure WiFi if needed.
    </div>
  </div>
  <script>
    const COLORS = { hour: '#%CHOUR%', minute: '#%CMIN%', second: '#%CSEC%' };
    const NUM_LEDS = 60;
    const leds = [];
    const svg = document.getElementById('ledCircle');
    const cx = 150, cy = 150, rad = 120;
    for (let i = 0; i < NUM_LEDS; i++) {
      const ang = (i * 6 - 90) * Math.PI / 180;
      const c = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
      c.setAttribute('cx', cx + rad * Math.cos(ang));
      c.setAttribute('cy', cy + rad * Math.sin(ang));
      c.setAttribute('r', 6);
      c.setAttribute('fill', '#333');
      svg.appendChild(c);
      leds.push(c);
    }
    function updateClock(timeStr) {
      const parts = timeStr.split(':').map(Number);
      if (parts.length !== 3 || parts.some(isNaN)) return;
      const [h, m, s] = parts;
      const hourLed = (h % 12) * 5 + Math.floor(m / 12);
      leds.forEach((led, i) => {
        if (i === s) led.setAttribute('fill', COLORS.second);
        else if (i === m) led.setAttribute('fill', COLORS.minute);
        else if (i === hourLed) led.setAttribute('fill', COLORS.hour);
        else led.setAttribute('fill', '#333');
      });
      document.getElementById('clock').innerText = timeStr;
    }
    function fetchTime() {
      fetch('/api/time').then(r => r.text()).then(t => updateClock(t));
    }
    function syncTime() {
      fetch('/sync').then(() => fetchTime());
    }
    function testEffect(type) {
      fetch('/test_effect?type=' + type).then(() => alert('Triggered ' + type + ' effect.'));
    }
    fetchTime();
    setInterval(fetchTime, 1000);
  </script>
</body>
</html>
)rawliteral";

String toHexColor(uint32_t c) {
    char buf[10];
    sprintf(buf, "%06X", c);
    return String(buf);
}

uint32_t parseHexColor(String hexStr) {
    if (hexStr.startsWith("#")) hexStr.remove(0, 1);
    return strtol(hexStr.c_str(), NULL, 16);
}

void handleRoot(AsyncWebServerRequest *request) {
    String html = htmlTemplate;
    html.replace("%CHOUR%", toHexColor(colorHour));
    html.replace("%CMIN%", toHexColor(colorMinute));
    html.replace("%CSEC%", toHexColor(colorSecond));
    html.replace("%MINUTE_EFFECT%", String(minuteEffectDuration));
    html.replace("%HOUR_EFFECT%", String(hourEffectDuration));
    html.replace("%GMT_OFFSET%", String(gmtOffsetHours));
    request->send(200, "text/html", html);
}

void handleGetTime(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", timeManager.getTimeString());
}

void handleSync(AsyncWebServerRequest *request) {
    timeManager.syncTime();
    request->send(200, "text/plain", timeManager.getTimeString());
}

void handleTestEffect(AsyncWebServerRequest *request) {
    if (request->hasParam("type")) {
        String type = request->getParam("type")->value();
        if (type == "minute") {
            display.triggerEffect(STATE_MINUTE_EFFECT);
        } else if (type == "hour") {
            display.triggerEffect(STATE_HOUR_EFFECT);
        }
    }
    request->send(200, "text/plain", "OK");
}

void handleSave(AsyncWebServerRequest *request) {
    if (request->hasParam("chour", true)) colorHour = parseHexColor(request->getParam("chour", true)->value());
    if (request->hasParam("cmin", true)) colorMinute = parseHexColor(request->getParam("cmin", true)->value());
    if (request->hasParam("csec", true)) colorSecond = parseHexColor(request->getParam("csec", true)->value());
    if (request->hasParam("minute_effect", true)) minuteEffectDuration = request->getParam("minute_effect", true)->value().toInt();
    if (request->hasParam("hour_effect", true)) hourEffectDuration = request->getParam("hour_effect", true)->value().toInt();
    if (request->hasParam("gmt_offset", true)) gmtOffsetHours = request->getParam("gmt_offset", true)->value().toInt();
    saveSettings();
    request->send(200, "text/html", "Settings saved. Rebooting...");
    delay(1000);
    ESP.restart();
}

extern bool enterWiFiSetup;

void handleConnectWifi(AsyncWebServerRequest *request) {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta http-equiv="refresh" content="3;url=http://192.168.4.1/">
  <style>body{font-family:sans-serif;margin:2rem;background:#222;color:#eee;text-align:center;}</style>
</head>
<body>
  <h2>Entering WiFi Config Mode...</h2>
  <p>If you are connected to the "LED Clock" WiFi, you will be redirected automatically.</p>
  <p>Otherwise, please manually connect your device to the WiFi network <b>LED Clock</b> (Password: 12345678) to continue.</p>
</body>
</html>
)rawliteral";
    request->send(200, "text/html", html);
    enterWiFiSetup = true;
}

// Captive portal redirect handling
void handleNotFound(AsyncWebServerRequest *request) {
    if (request->host() != WiFi.softAPIP().toString() && 
        request->host() != WiFi.localIP().toString()) {
        AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
        response->addHeader("Location", String("http://") + WiFi.softAPIP().toString());
        request->send(response);
        return;
    }
    request->send(404, "text/plain", "Not Found");
}

void setupWebUI() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/api/time", HTTP_GET, handleGetTime);
    server.on("/sync", HTTP_GET, handleSync);
    server.on("/test_effect", HTTP_GET, handleTestEffect);
    server.on("/connectwifi", HTTP_GET, handleConnectWifi);
    server.onNotFound(handleNotFound); // Capture captive portal requests
    server.begin();
}

void stopWebUI() {
    server.end();
}
