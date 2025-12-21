let ws = null;
let flag = -1;
let foundServers = [];
let currentConnMode = 'manual'; // Mặc định là manual

// --- HÀM CHUYỂN ĐỔI TAB (MỚI) ---
function switchConnMode(mode) {
    currentConnMode = mode;

    // 1. Xử lý giao diện nút Tab
    document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));

    if (mode === 'scan') {
        document.getElementById('tabScan').classList.add('active');
        document.getElementById('modeScan').style.display = 'block';
        document.getElementById('modeManual').style.display = 'none';
    } else {
        document.getElementById('tabManual').classList.add('active');
        document.getElementById('modeScan').style.display = 'none';
        document.getElementById('modeManual').style.display = 'block';
    }
}

// --- UTILS UI ---
function updateStatus(status, color) {
    const el = document.getElementById("connectionStatus");
    el.innerHTML = `<span class="dot" style="color: ${color}"></span> ${status}`;
    el.style.color = color;
}

// --- SCANNER ---
function scanNetwork() {
    const subnet = document.getElementById("subnetInput").value.trim();
    const port = document.getElementById("serverPort").value;
    const scanStatus = document.getElementById("scanStatus");
    const serverSelect = document.getElementById("serverList");

    if (!subnet.endsWith(".")) {
        alert("Subnet phải kết thúc bằng dấu chấm (VD: 192.168.1.)");
        return;
    }

    foundServers = [];
    serverSelect.innerHTML = '<option disabled selected>Scanning...</option>';
    scanStatus.style.display = "block";
    scanStatus.innerHTML = '<i class="fa-solid fa-spinner fa-spin"></i> Scanning network range...';

    let pendingChecks = 0;
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

function checkServer(ip, port, doneCallback) {
    let string = "ws://" + ip + ":" + port;
    let testWS = new WebSocket(string);
    let isConnected = false;

    testWS.onopen = () => {
        console.log("FOUND: " + ip);
        isConnected = true;
        foundServers.push(ip);
        testWS.close();
    };
    testWS.onerror = () => { };
    testWS.onclose = () => {
        if (doneCallback) { doneCallback(); doneCallback = null; }
    };
    setTimeout(() => {
        if (!isConnected && testWS.readyState !== WebSocket.OPEN) { testWS.close(); }
    }, 5000);
}

function updateDropdown() {
    const select = document.getElementById("serverList");
    select.innerHTML = "";
    if (foundServers.length === 0) {
        let option = document.createElement("option");
        option.text = "-- No Targets Found --"; option.disabled = true; option.selected = true; select.add(option);
    } else {
        foundServers.forEach(ip => {
            let option = document.createElement("option");
            option.value = ip; option.text = ip; select.add(option);
        });
        select.selectedIndex = 0;
    }
}

// --- CONNECTION ---
function connectToSelected() {
    const btn = document.getElementById("btnConnect");
    const port = document.getElementById("serverPort").value;
    let ip = "";

    // 1. Lấy IP (giữ nguyên logic của bạn)
    if (currentConnMode === 'scan') {
        const select = document.getElementById("serverList");
        ip = select.value;
        if (!ip) { alert("Please select a target!"); return; }
    } else {
        ip = document.getElementById("manualIpInput").value.trim();
        if (!ip) { alert("Enter IP!"); return; }
    }

    // 2. LOGIC ĐÓNG SOCKET CŨ (SỬA Ở ĐÂY)
    // Nếu đang có kết nối (dù là đang nối hay đã nối), đóng nó ngay lập tức
    if (ws && (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING)) {
        console.log("Closing existing connection to: ", ws.url);
        ws.close(); 
        // Không return ở đây nữa nếu bạn muốn bấm "Connect" là nó tự chuyển server luôn.
        // Nhưng để an toàn cho UI, ta đợi nó đóng xong hẳn rồi mới nối cái mới (xử lý ở bước 3).
    }

    // 3. Nếu nút đang là "Disconnect" (tức là người dùng muốn tắt), thì dừng lại sau khi close
    if (btn.classList.contains("connected")) {
        // ws.close() đã được gọi ở trên, ở đây chỉ cần return để kết thúc hàm
        return; 
    }

    // 4. Tạo kết nối mới
    connectToServer(ip, port);
}

function connectToServer(ip, port) {
    const url = `ws://${ip}:${port}`;
    updateStatus("Connecting...", "#f1c40f"); // Yellow

    try {
        ws = new WebSocket(url);
        ws.binaryType = "arraybuffer";
        ws.onopen = onWSOpen;
        ws.onclose = onWSClose;
        ws.onmessage = onWSMessage;
        ws.onerror = onWSError;
    } catch (e) { console.error(e); alert("Connection Failed!"); }
}

function onWSOpen() {
    updateStatus("ONLINE", "#00fff5"); // Cyan/Green
    const btn = document.getElementById("btnConnect");
    btn.innerHTML = '<i class="fa-solid fa-unlink"></i> DISCONNECT';
    btn.classList.add("connected");
    document.getElementById("serverList").disabled = true;
    document.getElementById("subnetInput").disabled = true;

    const dot = document.getElementsByClassName('dot')[0];
    dot.style.backgroundColor = "#00ff37ff";
}

function onWSClose() {
    updateStatus("OFFLINE", "#ff2e63");
    const btn = document.getElementById("btnConnect");
    btn.innerHTML = '<i class="fa-solid fa-link"></i> CONNECT';
    btn.classList.remove("connected");
    document.getElementById("serverList").disabled = false;
    document.getElementById("subnetInput").disabled = false;
    ws = null;
}

function onWSError(err) { console.error("WS Error:", err); }

// --- MESSAGE HANDLER ---
function onWSMessage(event) {
    if (event.data instanceof ArrayBuffer) {
        if (flag == 1) { // Screenshot
            const blob = new Blob([event.data], { type: "image/bmp" });
            const url = URL.createObjectURL(blob);
            document.getElementById("screenshotContainer").innerHTML = `<img src="${url}">`;
        }
        else if (flag == 2) { // Webcam
            const blob = new Blob([event.data], { type: 'video/mp4' });
            const videoUrl = URL.createObjectURL(blob);
            const videoPlayer = document.getElementById('videoPlayer');
            if (videoPlayer.src) URL.revokeObjectURL(videoPlayer.src);
            videoPlayer.src = videoUrl;
            videoPlayer.play().catch(e => console.error(e));
        }
        flag = -1;
    } else {
        HandleClientMSG(event.data);
    }
}

function HandleClientMSG(data) {
    console.log(data);
    if (data == "screenshot") { flag = 1; return; }
    if (data == "webcam") { flag = 2; return; }
    if (data == "Keylogging started.") { flag = 3; logKeyToConsole(">>> [SYSTEM] Keylogger Started"); return; }
    if (data == "Keylogging stopped.") { flag = 4; logKeyToConsole(">>> [SYSTEM] Keylogger Stopped"); return; }
    if (data === "Keylog") {
        flag = 7;
        console.log("WTF");
        return;
    }

    if (flag === 5) renderProcessListToTable(data);
    else if (flag == 6) renderAppListToTable(data);
    else if (flag == 7) logKeyToConsole(data);
    else if (flag == 8) alert(data);

    if (flag != 3) flag = -1;
}

function sendCommand(cmd) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(cmd);
    } else {
        alert("Not Connected!");
    }
}

function switchTab(tabId) {
    document.querySelectorAll('.content-section').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.nav-links li').forEach(el => el.classList.remove('active'));
    document.getElementById('tab-' + tabId).classList.add('active');
    event.currentTarget.classList.add('active');
}

// --- APP & PROCESS ---
function listApp() { flag = 6; sendCommand("list_apps"); }

function renderAppListToTable(dataString) {
    const tbody = document.getElementById("appListBody");
    tbody.innerHTML = "";
    const apps = dataString.split('\n');

    apps.forEach(line => {
        if (line.trim().length > 0) {
            const tr = document.createElement("tr");
            const parts = line.split('|');
            const name = parts[0].trim();
            const path = parts.length > 1 ? parts[1].trim() : "";

            // 1. Cột Tên App
            const tdName = document.createElement("td");
            tdName.innerHTML = `<strong style="color: #fff; font-size: 13px;">${name}</strong><br><span style="font-size: 11px; color: #666; font-family: 'JetBrains Mono', monospace;">${path || 'Unknown Path'}</span>`;

            // 2. Cột Control (Action)
            const tdAction = document.createElement("td");

            // --- SỬA LỖI TẠI ĐÂY: TẠO DIV BAO BỌC ---
            const divGroup = document.createElement("div");
            divGroup.className = "btn-group-horizontal"; // Class CSS mới thêm

            // Logic xác định target
            const startTarget = (path && path !== "Not Found") ? path : name;
            let stopTarget = name;
            if (path && path.includes("\\")) stopTarget = path.split('\\').pop();
            else if (!stopTarget.toLowerCase().endsWith(".exe")) stopTarget += ".exe";

            // Tạo nút
            const btnStart = createBtn('Start', "btn-neon-green", () => {
                flag = 8;
                sendCommand("start_app:" + startTarget)
            });
            const btnStop = createBtn('Kill', "btn-neon-red", () => {
                flag = 8;
                sendCommand("stop_app:" + stopTarget)
            });

            // Thêm nút vào DIV Group trước
            divGroup.appendChild(btnStart);
            divGroup.appendChild(btnStop);

            // Thêm DIV Group vào ô bảng
            tdAction.appendChild(divGroup);

            tr.appendChild(tdName);
            tr.appendChild(tdAction);
            tbody.appendChild(tr);
        }
    });
}

function manualStart() {
    const name = document.getElementById("manualAppName").value; if (name) { flag = 8; sendCommand("start_app:" + name); }
}
function manualStop() {
    const name = document.getElementById("manualAppName").value; if (name) { flag = 8; sendCommand("stop_app:" + name); }
}
function listProcess() {
    if (ws && ws.readyState === WebSocket.OPEN) {
        flag = 5;
        document.getElementById("processListBody").innerHTML = '<tr><td colspan="4" style="text-align:center; color:#ff9f43;">Fetching process list...</td></tr>';
        ws.send("list_processes");
    } else alert("Not Connected!");
}

function renderProcessListToTable(dataString) {
    const tbody = document.getElementById("processListBody");
    tbody.innerHTML = "";
    const regex = /PID:\s*(\d+),\s*Name:\s*(.*?),\s*Threads:\s*(\d+)/g;
    let match;
    let count = 0;

    while ((match = regex.exec(dataString)) !== null) {
        count++;
        const pid = match[1]; const name = match[2]; const threads = match[3];
        const tr = document.createElement("tr");

        tr.innerHTML = `
            <td style="font-family: monospace; color: var(--primary)">${pid}</td>
            <td><strong>${name}</strong></td>
            <td style="color: #aaa">${threads}</td>
        `;

        const tdAction = document.createElement("td");

        // --- CŨNG BỌC DIV CHO PROCESS ---
        const divGroup = document.createElement("div");
        divGroup.className = "btn-group-horizontal";

        const btnKill = createBtn('Kill', "btn-neon-red", () => {
            if (confirm(`Kill ${name} (PID: ${pid})?`)) {
                flag = 8;
                sendCommand("stop_process:" + pid);
            }
        });

        divGroup.appendChild(btnKill);
        tdAction.appendChild(divGroup);
        tr.appendChild(tdAction);

        tbody.appendChild(tr);
    }
    if (count === 0) tbody.innerHTML = '<tr><td colspan="4" class="empty-state">No Data Found</td></tr>';
}

function manualStopProc() {
    const pid = document.getElementById("manualProcID").value; if (pid) { flag = 8; sendCommand("stop_process:" + pid); }
}

function createBtn(text, className, onClick) {
    const btn = document.createElement("button");
    btn.className = "btn " + className;
    btn.innerText = text;
    btn.style.marginRight = "5px";
    btn.style.padding = "4px 10px"; // Smaller in table
    btn.style.fontSize = "11px";
    btn.onclick = onClick;
    return btn;
}

// --- MEDIA & LOG ---
function startkeyLog() { sendCommand("start_keylog"); }
function stopkeyLog() { sendCommand("stop_keylog"); }
function screenShot() { sendCommand("screenshot"); }
function webCam() { sendCommand("webcam"); }

// Biến lưu trữ dòng hiện tại
let currentLogLine = null;

function logKeyToConsole(msg) {
    const consoleBox = document.getElementById("keylogConsole");

    // Nếu chưa có dòng hiện tại hoặc gặp phím ENTER, tạo một dòng mới
    if (!currentLogLine) {
        // Tạo dòng mới (div)
        const div = document.createElement("div");
        div.className = "console-line";

        // Thêm timestamp cho dòng mới
        const time = new Date().toLocaleTimeString('en-US', { hour12: false });
        div.innerHTML = `<span style="color: #555">[${time}]</span> `;

        consoleBox.appendChild(div);
        currentLogLine = div; // Gán dòng này là dòng hiện tại để các key sau nối vào
    }

    // Nếu không phải ENTER, nối key vào dòng hiện tại
    if (currentLogLine) {
        // Tạo span để bọc ký tự cho đẹp (tùy chọn)
        const keySpan = document.createElement("span");
        if (currentLogLine.textContent.includes(">>> [SYSTEM] Keylogger Started")) {
            // Tạo dòng mới (div)
            const div = document.createElement("div");
            div.className = "console-line";

            // Thêm timestamp cho dòng mới
            const time = new Date().toLocaleTimeString('en-US', { hour12: false });
            div.innerHTML = `<span style="color: #555">[${time}]</span> `;

            consoleBox.appendChild(div);
            currentLogLine = div; // Gán dòng này là dòng hiện tại để các key sau nối vào
        }
        if (msg.includes(">>> [SYSTEM] Keylogger Stopped")) {
            // Tạo dòng mới (div)
            const div = document.createElement("div");
            div.className = "console-line";

            // Thêm timestamp cho dòng mới
            const time = new Date().toLocaleTimeString('en-US', { hour12: false });
            div.innerHTML = `<span style="color: #555">[${time}]</span> `;

            consoleBox.appendChild(div);
            currentLogLine = div; // Gán dòng này là dòng hiện tại để các key sau nối vào
            keySpan.innerText = msg;
            currentLogLine.appendChild(keySpan);
            currentLogLine = null; // Reset dòng hiện tại
            return;
        }

        // Nếu là các phím chức năng (nằm trong ngoặc vuông []), cho màu khác
        if (msg.startsWith("[") && msg.endsWith("]")) {
            keySpan.style.color = "#00ff88"; // Màu xanh cho phím chức năng
            keySpan.innerText = ` ${msg} `;
        } else {
            keySpan.innerText = msg;
        }

        currentLogLine.appendChild(keySpan);
    }

    // Luôn cuộn xuống dưới cùng
    consoleBox.scrollTop = consoleBox.scrollHeight;
}

// Cập nhật lại hàm clearConsole để reset biến dòng hiện tại
function clearConsole() {
    document.getElementById("keylogConsole").innerHTML = '<div class="console-line system-msg">>> Console cleared.<span class="cursor">_</span></div>';
    currentLogLine = null;
}

// --- SYSTEM CONTROL LOGIC ---
function confirmSystem(action) {
    if (!ws || ws.readyState !== WebSocket.OPEN) {
        alert("Not Connected to Server!");
        return;
    }

    const msg = action === 'shutdown' ?
        "WARNING: This will SHUTDOWN the remote server immediately. Continue?" :
        "Confirm RESTART of the remote server?";

    if (confirm(msg)) {
        // Gửi chính xác chuỗi mà Backend C++ đang đợi (như trong ảnh bạn cung cấp)
        if (action === 'shutdown') {
            sendCommand("shutdown");
        } else {
            sendCommand("restart");
        }

        // Thông báo cho người dùng biết lệnh đã gửi
        logKeyToConsole(`>>> [SYSTEM] Sent ${action.toUpperCase()} command.`);
    }
}

window.onbeforeunload = function() {
    if (ws) {
        ws.onclose = function () {}; // Tắt sự kiện onclose để không kích hoạt UI update khi trang đang đóng
        ws.close();
    }
};