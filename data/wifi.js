async function fetchWifiList() {
  try {
    const res = await fetch("/scan");
    return await res.json();
  } catch (err) {
    return [];
  }
}

function renderWifiList(networks) {
  const container = document.getElementById("wifiContainer");
  container.innerHTML = "";
  const list = document.createElement("div");
  list.className = "wifi-list";
  networks.forEach(ssid => {
    const btn = document.createElement("button");
    btn.className = "ssid-btn";
    btn.type = "button";
    btn.textContent = ssid;
    btn.addEventListener("click", () => {
      document.getElementById("ssidInput").value = ssid;
    });
    list.appendChild(btn);
  });
  container.appendChild(list);
}

function startWifiScan() {
  const container = document.getElementById("wifiContainer");
  if (container.textContent !== "Đang tải...") container.textContent = "Đang quét WiFi...";
  fetchWifiList().then(networks => {
    // Nếu mảng rỗng, có thể đang quét ngầm (WIFI_SCAN_RUNNING) hoặc không có. Poll lại sau 2s.
    if (!networks.length) {
      setTimeout(startWifiScan, 2000);
      return;
    }
    renderWifiList(networks);
  }).catch(() => {
    container.textContent = "Lỗi quét WiFi. Đang thử lại...";
    setTimeout(startWifiScan, 2000);
  });
}

function bindWifiForm() {
  const form = document.getElementById("wifiForm");
  const btn = document.getElementById("btn");
  const result = document.getElementById("result");
  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    btn.disabled = true;
    result.textContent = "Đang kết nối...";
    const body = new URLSearchParams(new FormData(form));
    try {
      const resp = await fetch("/connect", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body
      });
      const data = await resp.json();
      result.textContent = data.message || "Đã gửi yêu cầu kết nối...";
    } catch (err) {
      result.textContent = "Lỗi gửi cấu hình WiFi.";
    } finally {
      btn.disabled = false;
    }
  });
}

// Entrypoint
document.addEventListener("DOMContentLoaded", () => {
  startWifiScan();
  bindWifiForm();
});
