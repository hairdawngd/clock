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

  const body = new URLSearchParams(new FormData(form));
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
  fetchcurrentTime();
});
