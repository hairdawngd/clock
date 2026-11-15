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

let connecting = false;   // đang thử kết nối thì không cần cập nhật danh sách quét

function startWifiScan() {
  const container = document.getElementById("wifiContainer");
  fetchWifiList().then(networks => {
    // Mảng rỗng: đang quét ngầm, hoặc server đang bận thử kết nối. Poll lại sau 2s.
    if (!networks.length) {
      setTimeout(startWifiScan, 2000);
      return;
    }
    if (!connecting) renderWifiList(networks);
  }).catch(() => {
    container.textContent = "Lỗi quét WiFi. Đang thử lại...";
    setTimeout(startWifiScan, 2000);
  });
}

function pollStatus(result, btn, tries = 0) {
  fetch("/connect_status")
    .then(r => r.json())
    .then(s => {
      if (s.state === "connecting" && tries < 25) {
        result.textContent = "Đang kết nối...";
        setTimeout(() => pollStatus(result, btn, tries + 1), 1000);
        return;
      }
      if (s.state === "success") {
        result.textContent = "Kết nối thành công! IP: " + s.ip +
          ". Thiết bị đang khởi động lại, hãy kết nối lại mạng WiFi nhà bạn.";
        return;   // giữ nút bị khóa vì thiết bị sắp restart
      }
      // failed, idle hoặc quá thời gian chờ
      connecting = false;
      result.textContent = "Sai mật khẩu hoặc không kết nối được. Vui lòng kiểm tra và nhập lại.";
      btn.disabled = false;
      document.getElementById("pwdInput").focus();
    })
    .catch(() => {
      // Mất kết nối tới ESP32: có thể vừa restart sau khi thành công
      result.textContent = "Mất kết nối tới thiết bị. Nếu bạn đã nhập đúng, thiết bị có thể đang khởi động lại.";
      btn.disabled = false;
      connecting = false;
    });
}

function bindWifiForm() {
  const form = document.getElementById("wifiForm");
  const btn = document.getElementById("btn");
  const result = document.getElementById("result");
  form.addEventListener("submit", async (e) => {
    e.preventDefault();
    btn.disabled = true;
    connecting = true;
    result.textContent = "Đang kết nối...";
    const body = new URLSearchParams(new FormData(form));
    try {
      const resp = await fetch("/connect", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body
      });
      const data = await resp.json();
      if (!resp.ok) {
        result.textContent = data.error || "Lỗi gửi cấu hình WiFi.";
        btn.disabled = false;
        connecting = false;
        return;
      }
      pollStatus(result, btn);
    } catch (err) {
      result.textContent = "Lỗi gửi cấu hình WiFi.";
      btn.disabled = false;
      connecting = false;
    }
  });
}

// Entrypoint
document.addEventListener("DOMContentLoaded", () => {
  startWifiScan();
  bindWifiForm();
});