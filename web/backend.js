// KHAI BÁO BIẾN TOÀN CỤC
let ws = null;
let flag = -1;
let foundServers = [];

// --- PHẦN 1: QUẢN LÝ QUÉT MẠNG (SCANNER) ---
// (Giữ nguyên code phần Scan như cũ)
function scanNetwork() {
    const subnet = document.getElementById("subnetInput").value.trim();
    const port = document.getElementById("serverPort").value;
    const scanStatus = document.getElementById("scanStatus");
    const serverSelect = document.getElementById("serverList");

    if (!subnet.endsWith(".")) {
        alert("Subnet phải kết thúc bằng dấu chấm (VD: 192.168.2.)");
        return;
    }

    foundServers = [];
    serverSelect.innerHTML = '<option disabled selected>Scanning...</option>';
    scanStatus.style.display = "block";

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
        console.log("FOUND SERVER AT: " + ip);
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
    }, 5000); // Giảm timeout xuống 5s cho nhanh
}

function updateDropdown() {
    const select = document.getElementById("serverList");
    select.innerHTML = "";
    if (foundServers.length === 0) {
        let option = document.createElement("option");
        option.text = "-- No Server Found --"; option.disabled = true; option.selected = true; select.add(option);
    } else {
        foundServers.forEach(ip => {
            let option = document.createElement("option");
            option.value = ip; option.text = ip; select.add(option);
        });
        select.selectedIndex = 0;
    }
}

// --- PHẦN 2: KẾT NỐI ---
function connectToSelected() {
    const btn = document.getElementById("btnConnect");
    const select = document.getElementById("serverList");
    const port = document.getElementById("serverPort").value;

    if (ws && (ws.readyState === WebSocket.OPEN || ws.readyState === WebSocket.CONNECTING)) {
        ws.close(); return;
    }
    const ip = select.value;
    if (!ip) { alert("Vui lòng Scan và chọn 1 Server từ danh sách!"); return; }
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
    } catch (e) { console.error("Connection Error:", e); alert("Lỗi kết nối!"); }
}

function onWSOpen() {
    document.getElementById("connectionStatus").innerHTML = "🟢 Connected";
    document.getElementById("connectionStatus").style.color = "#28a745";
    document.getElementById("btnConnect").innerText = "Disconnect";
    document.getElementById("btnConnect").classList.add("connected");
    document.getElementById("serverList").disabled = true;
    document.getElementById("subnetInput").disabled = true;
}

function onWSClose() {
    document.getElementById("connectionStatus").innerHTML = "🔴 Disconnected";
    document.getElementById("connectionStatus").style.color = "#dc3545";
    document.getElementById("btnConnect").innerText = "Connect";
    document.getElementById("btnConnect").classList.remove("connected");
    document.getElementById("serverList").disabled = false;
    document.getElementById("subnetInput").disabled = false;
    ws = null;
}

function onWSError(err) { console.error("WebSocket Error:", err); }

// --- PHẦN 3: XỬ LÝ MESSAGE ---

function onWSMessage(event) {
    // 1. Binary Data (Image/Video)
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
    }
    // 2. Text Data
    else {
        HandleClientMSG(event.data);
    }
}

function HandleClientMSG(data) {
    // Flag: 1=Screen, 2=Cam, 3=KeylogStart, 4=KeylogStop, 5=ProcessList

    if (data == "screenshot") { flag = 1; return; }
    if (data == "webcam") { flag = 2; return; }
    if (data == "Keylogging started") { flag = 3; logKeyToConsole(">>> Keylogger Started"); return; }
    if (data == "Keylogging stopped") { flag = 4; logKeyToConsole(">>> Keylogger Stopped"); return; }

    // XỬ LÝ LIST PROCESS (Flag 5)
    if (flag === 5) {
        renderProcessListToTable(data);
    }

    // XỬ LÝ LIST APP (Nhận diện bằng đuôi .exe)
    else if (flag == 6) {
        renderAppListToTable(data);
    } else { 
        logKeyToConsole(data);
    }

    if (flag != 3) flag = -1;
}

// --- CÁC HÀM GỬI LỆNH ---

function sendCommand(cmd) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(cmd);
        console.log("Sent:", cmd);
    } else {
        alert("Chưa kết nối!");
    }
}

function switchTab(tabId) {
    document.querySelectorAll('.content-section').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.nav-links li').forEach(el => el.classList.remove('active'));
    document.getElementById('tab-' + tabId).classList.add('active');
    event.currentTarget.classList.add('active');
}

// --- LOGIC APP MANAGER ---
function listApp() { flag = 6; sendCommand("list_apps"); }

//function renderAppListToTable(dataString) {
//    const tbody = document.getElementById("appListBody");
//    tbody.innerHTML = "";
//    const apps = dataString.split('\n');
//    apps.forEach(app => {
//        if (app.trim().length > 0) {
//            const tr = document.createElement("tr");

//            // Cột tên
//            const tdName = document.createElement("td");
//            tdName.textContent = app;

//            // Cột nút bấm
//            const tdAction = document.createElement("td");
//            const btnStart = createBtn("Start", "btn-success", () => sendCommand("start_app:" + app.trim()));
//            const btnStop = createBtn("End", "btn-danger", () => sendCommand("stop_app:" + app.trim()));
//            tdAction.appendChild(btnStart); tdAction.appendChild(btnStop);

//            tr.appendChild(tdName); tr.appendChild(tdAction); tbody.appendChild(tr);
//        }
//    });
//}

//function renderAppListToTable(dataString) {
//    const tbody = document.getElementById("appListBody");
//    tbody.innerHTML = "";
//    const apps = dataString.split('\n');

//    apps.forEach(line => {
//        // Kiểm tra dòng không rỗng
//        if (line.trim().length > 0) {
//            const tr = document.createElement("tr");

//            // --- XỬ LÝ CHUỖI MỚI (SPLIT) ---
//            // Định dạng: "Name|Path"
//            const parts = line.split('|');
//            const name = parts[0].trim();
//            // Lấy path nếu có, nếu không thì gán chuỗi rỗng
//            const path = parts.length > 1 ? parts[1].trim() : "";

//            // --- CỘT TÊN (HIỂN THỊ) ---
//            const tdName = document.createElement("td");

//            // Cách 1: Chỉ hiện tên, Path hiện khi di chuột vào (Tooltip) -> Gọn gàng
//            //tdName.textContent = name;
//            //if (path && path !== "Not Found") {
//            //    tdName.title = path; // Tooltip
//            //}

//             // Cách 2: (Tùy chọn) Hiện Path ngay dưới tên -> Chi tiết hơn
//            if (path && path !== "Not Found") {
//                tdName.innerHTML = `<strong>${name}</strong><br><span style="font-size: 0.8em; color: gray;">${path}</span>`;
//            } else {
//                tdName.textContent = name;
//            }
            

//            // --- CỘT ACTION (NÚT BẤM) ---
//            const tdAction = document.createElement("td");

//            // Logic nút Start: 
//            // Nếu có Path hợp lệ thì gửi Path (để backend ShellExecute chính xác file đó).
//            // Nếu không có Path thì gửi Name (để backend tìm trong ổ C/D như cũ).
//            const runTarget = (path && path !== "Not Found" && path !== "Unknown Path") ? path : name;

//            const btnStart = createBtn("Start", "btn-success", () => sendCommand("start_app:" + runTarget));

//            // Logic nút Stop:
//            // Thường stop process bằng Tên (VD: notepad.exe) an toàn và dễ hơn dùng đường dẫn đầy đủ.
//            const btnStop = createBtn("End", "btn-danger", () => sendCommand("stop_app:" + name));

//            tdAction.appendChild(btnStart);
//            tdAction.appendChild(btnStop);

//            tr.appendChild(tdName);
//            tr.appendChild(tdAction);
//            tbody.appendChild(tr);
//        }
//    });
//}

function renderAppListToTable(dataString) {
    const tbody = document.getElementById("appListBody");
    tbody.innerHTML = "";
    const apps = dataString.split('\n');

    apps.forEach(line => {
        if (line.trim().length > 0) {
            const tr = document.createElement("tr");

            // 1. Parse dữ liệu: "Name|Path"
            const parts = line.split('|');
            const name = parts[0].trim();
            const path = parts.length > 1 ? parts[1].trim() : "";

            // --- CỘT TÊN ---
            const tdName = document.createElement("td");
            tdName.textContent = name;
            if (path && path !== "Not Found") {
                tdName.innerHTML = `<strong>${name}</strong><br><span style="font-size: 0.8em; color: gray;">${path}</span>`;
            } else {
                tdName.textContent = name;
            }

            // --- CỘT ACTION ---
            const tdAction = document.createElement("td");

            // LOGIC START: Ưu tiên gửi Path đầy đủ để run chính xác
            const startTarget = (path && path !== "Not Found") ? path : name;

            // LOGIC STOP: Cắt lấy tên file .exe từ Path
            let stopTarget = name; // Mặc định là Name

            // Nếu có path hợp lệ (chứa dấu \)
            if (path && path.includes("\\")) {
                // Cắt chuỗi theo dấu \ và lấy phần tử cuối cùng
                // VD: "C:\Program Files\App\my_app.exe" -> "my_app.exe"
                stopTarget = path.split('\\').pop();
            }
            // Nếu không có path, ta có thể tự động thêm .exe vào name (tùy chọn)
            else {
                 if (!stopTarget.toLowerCase().endsWith(".exe")) {
                     stopTarget += ".exe";
                 }
            } 

            const btnStart = createBtn("Start", "btn-success", () => sendCommand("start_app:" + startTarget));
            const btnStop = createBtn("End", "btn-danger", () => sendCommand("stop_app:" + stopTarget));

            tdAction.appendChild(btnStart);
            tdAction.appendChild(btnStop);

            tr.appendChild(tdName);
            tr.appendChild(tdAction);
            tbody.appendChild(tr);
        }
    });
}
function manualStart() { const name = document.getElementById("manualAppName").value; if (name) sendCommand("start_app:" + name); }
function manualStop() { const name = document.getElementById("manualAppName").value; if (name) sendCommand("stop_app:" + name); }


// --- LOGIC PROCESS MANAGER (MỚI) ---

function listProcess() {
    if (ws && ws.readyState === WebSocket.OPEN) {
        flag = 5; // Đặt cờ báo hiệu dữ liệu sắp tới là Process List
        // Hiển thị trạng thái đang tải vào bảng
        document.getElementById("processListBody").innerHTML = '<tr><td colspan="4" style="text-align:center; color:orange;">Fetching data...</td></tr>';
        ws.send("list_processes");
    } else {
        alert("Chưa kết nối!");
    }
}

// Hàm parse chuỗi: "PID: 123, Name: abc.exe, Threads: 4, PID: 456..."
function renderProcessListToTable(dataString) {
    const tbody = document.getElementById("processListBody");
    tbody.innerHTML = "";

    // Regex giải thích: Tìm cụm "PID: (số), Name: (chữ), Threads: (số)"
    // /g để tìm tất cả các cụm trùng khớp trong chuỗi
    const regex = /PID:\s*(\d+),\s*Name:\s*(.*?),\s*Threads:\s*(\d+)/g;
    let match;
    let count = 0;

    // Lặp qua từng kết quả tìm thấy
    while ((match = regex.exec(dataString)) !== null) {
        count++;
        const pid = match[1];      // Group 1: ID
        const name = match[2];     // Group 2: Name
        const threads = match[3];  // Group 3: Threads

        const tr = document.createElement("tr");

        // PID
        const tdId = document.createElement("td");
        tdId.textContent = pid;

        // Name
        const tdName = document.createElement("td");
        tdName.textContent = name;
        tdName.style.fontWeight = "bold";

        // Threads
        const tdThread = document.createElement("td");
        tdThread.textContent = threads;

        // Action (Kill Button)
        const tdAction = document.createElement("td");
        const btnKill = createBtn("Kill", "btn-danger", () => {
            if (confirm(`Kill process ${name} (ID: ${pid})?`)) {
                sendCommand("stop_process:" + pid);
            }
        });
        tdAction.appendChild(btnKill);

        tr.appendChild(tdId);
        tr.appendChild(tdName);
        tr.appendChild(tdThread);
        tr.appendChild(tdAction);

        tbody.appendChild(tr);
    }

    if (count === 0) {
        tbody.innerHTML = '<tr><td colspan="4" style="text-align:center">No processes found or parse error.</td></tr>';
    }
}

function manualStopProc() {
    const pid = document.getElementById("manualProcID").value;
    if (pid) sendCommand("stop_process:" + pid);
}

// Helper tạo nút nhanh
function createBtn(text, className, onClick) {
    const btn = document.createElement("button");
    btn.className = "btn " + className;
    btn.innerText = text;
    btn.style.marginRight = "5px";
    btn.onclick = onClick;
    return btn;
}

// Shutdown and Restart
function confirmAction(action) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        if (confirm("Are you sure you want to " + action.toUpperCase() + " the remote server?")) {
            sendCommand(action);
        }
    } else {
        alert("Chưa kết nối!");
    }
}

// --- LOGIC KEYLOGGER, SCREENSHOT, WEBCAM ---
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