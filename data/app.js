async function fetchSettings() {
  try {
    const res = await fetch("/api/settings");
    return await res.json();
  } catch (err) {
    console.error("Lỗi tải cài đặt", err);
    return {};
  }
}

function fillForm(data) {
  document.getElementById("chour").value = data.chour;
  document.getElementById("cmin").value = data.cmin;
  document.getElementById("csec").value = data.csec;
  document.getElementById("cmark").value = data.cmark;
  document.getElementById("minute_effect").value = data.minute_effect;
  document.getElementById("hour_effect").value = data.hour_effect;
  document.getElementById("effect_start").value = data.effect_start ?? "00:00";
  document.getElementById("effect_end").value = data.effect_end ?? "23:59";
  document.getElementById("gmt_offset").value = data.gmt_offset;
  document.getElementById("brightness").value = data.brightness ?? 255;
  document.getElementById("brightness_value").textContent = data.brightness ?? 255;
  document.getElementById("eff_minute").value = data.eff_minute ?? 1;
  document.getElementById("eff_hour").value = data.eff_hour ?? 2;
  document.getElementById("display_enabled").checked = data.display_enabled ?? true;
  document.getElementById("background_enabled").checked = data.background_enabled ?? true;
  document.getElementById("background_color").value = data.background_color ?? "#050505";
  document.getElementById("background_effect").value = data.background_effect ?? 0;
  document.getElementById("background_effect_duration").value = data.background_effect_duration ?? 5000;
  document.getElementById("background_start").value = data.background_start ?? "00:00";
  document.getElementById("background_end").value = data.background_end ?? "23:59";
  document.getElementById("ota_password").value = "";
  document.getElementById("display_pin").value = data.display_pin ?? 3;
  document.getElementById("display_count").value = data.display_count ?? 70;
  document.getElementById("background_pin").value = data.background_pin ?? 4;
  document.getElementById("background_count").value = data.background_count ?? 60;
  document.getElementById("mqtt_enabled").checked = data.mqtt_enabled ?? false;
  document.getElementById("mqtt_discovery").checked = data.mqtt_discovery ?? true;
  document.getElementById("mqtt_host").value = data.mqtt_host ?? "homeassistant.local";
  document.getElementById("mqtt_port").value = data.mqtt_port ?? 1883;
  document.getElementById("mqtt_user").value = data.mqtt_user ?? "";
  document.getElementById("mqtt_topic").value = data.mqtt_topic ?? "clock-led";
}

function bindBrightnessLabels() {
  ["brightness"].forEach(id => {
    const input = document.getElementById(id);
    const output = document.getElementById(id + "_value");
    input.addEventListener("input", () => output.textContent = input.value);
  });
}

function bindLiveColorSync() {
  const cHourInput = document.getElementById("chour");
  const cMinInput = document.getElementById("cmin");
  const cSecInput = document.getElementById("csec");
  const cMarkInput = document.getElementById("cmark");
  let colorTimeout;
  const syncColor = () => {
    clearTimeout(colorTimeout);
    colorTimeout = setTimeout(() => {
      const body = new URLSearchParams();
      body.append("chour", cHourInput.value);
      body.append("cmin", cMinInput.value);
      body.append("csec", cSecInput.value);
      body.append("cmark", cMarkInput.value);
      fetch("/api/test_color", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body
      }).catch(err => console.error(err));
    }, 200);
  };
  cHourInput.addEventListener("input", syncColor);
  cMinInput.addEventListener("input", syncColor);
  cSecInput.addEventListener("input", syncColor);
  cMarkInput.addEventListener("input", syncColor);
}

async function saveForm(e) {
  e.preventDefault();
  const form = document.getElementById("setupForm");
  const btn = document.getElementById("btnSave");
  const result = document.getElementById("saveResult");
  btn.disabled = true;
  result.textContent = "Đang lưu...";

  const formData = new FormData(form);
  formData.set("display_enabled", document.getElementById("display_enabled").checked ? "1" : "0");
  formData.set("background_enabled", document.getElementById("background_enabled").checked ? "1" : "0");
  formData.set("mqtt_enabled", document.getElementById("mqtt_enabled").checked ? "1" : "0");
  formData.set("mqtt_discovery", document.getElementById("mqtt_discovery").checked ? "1" : "0");
  const body = new URLSearchParams(formData);
  try {
    const resp = await fetch("/api/save", {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body
    });
    const data = await resp.json();
    if (!resp.ok) {
      result.textContent = data.error || "Lỗi khi lưu cài đặt.";
      return;
    }
    result.textContent = data.message || "Lưu thành công, đang khởi động lại...";
    // Update values from backend again after save
    const s = await fetchSettings();
    fillForm(s);
  } catch (err) {
    result.textContent = "Lỗi khi lưu cài đặt.";
  } finally {
    btn.disabled = false;
  }
}

async function fetchcurrentTime() {
  try {
    const res = await fetch("/api/time");
    const data = await res.json();
    const el = document.getElementById("currentTime");
    if (el)
      el.textContent = (data.time || "--:--:--");
  } catch (e) {
    const el = document.getElementById("currentTime");
    if (el)
      el.textContent = "Không lấy được thời gian từ backend.";
  }
}
setInterval(fetchcurrentTime, 1000);

document.addEventListener("DOMContentLoaded", async () => {
  const settings = await fetchSettings();
  fillForm(settings);
  bindBrightnessLabels();
  bindLiveColorSync();
  document.getElementById("setupForm").addEventListener("submit", saveForm);
  document.getElementById("otaForm").addEventListener("submit", async (event) => {
    event.preventDefault();
    const form = event.target;
    const password = form.elements.password.value;
    const result = document.getElementById("otaResult");
    result.textContent = "Đang cập nhật...";
    try {
      const response = await fetch("/update", {
        method: "POST",
        headers: { Authorization: "Basic " + btoa("admin:" + password) },
        body: new FormData(form)
      });
      result.textContent = await response.text();
    } catch (error) {
      result.textContent = "Không thể cập nhật firmware.";
    }
  });
  fetchcurrentTime();
});
