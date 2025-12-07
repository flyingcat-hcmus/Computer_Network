let ws = new WebSocket("ws://192.168.2.152:9000"); // IP server
ws.binaryType = "arraybuffer";

ws.onopen = () => {
    log("Connected!");
    
};

// // Mỗi khi server gửi lại client bằng s->send thì
// // data được truyền vào "e"
ws.onmessage = (event) => {
    console.log("Đã nhận dữ liệu từ Server:", event.data); // Kiểm tra Console F12

    if (event.data instanceof ArrayBuffer) {
        console.log("--> Là dữ liệu nhị phân (Ảnh), kích thước:", event.data.byteLength);
        
        // Tạo ảnh
        let bytes = new Uint8Array(event.data);
        let blob = new Blob([bytes], { type: "image/bmp" });
        let url = URL.createObjectURL(blob);

        // Tìm thẻ ảnh cũ để cập nhật, nếu chưa có thì tạo mới
        let img = document.getElementById("anhManHinh");
        if (!img) {
            img = document.createElement("img");
            img.id = "anhManHinh";
            img.style.width = "80%"; // Chỉnh lại cho vừa màn hình
            img.style.border = "5px solid white"; // Viền đỏ cho dễ nhìn
            document.body.appendChild(img);
        }
        img.src = url;
    } else {
        console.log("--> Là tin nhắn văn bản:", event.data);
    }
};


function sendHello() {
    if (ws.readyState === WebSocket.OPEN) {
        ws.send("Hello");
    } 
    else {
        log("WebSocket chưa kết nối xong.");
    }
}

function log(msg) {
    const logEl = document.getElementById("log");
    if (logEl) {
        logEl.append(document.createTextNode(msg));
        logEl.append(document.createElement("br"));
    }
}

function listApp(){
    if (ws.readyState === WebSocket.OPEN){
        ws.send("list_apps");
    }
    else {
        log("WebSocket chưa kết nối xong.");
    }
}

function StartApp(event){

    event.preventDefault();

    const appNameInput = document.getElementById('startapp');
    const applicationName = appNameInput.value;

    if (ws.readyState === WebSocket.OPEN){
        ws.send("start_app:" + applicationName);
        appNameInput.value = ""
    }
    else {
        log("WebSocket chưa kết nối xong.");
    }
}
let startApp = document.getElementById("StartApp");
startApp.addEventListener('submit', StartApp);

function StopApp(event){
    event.preventDefault();

    const appNameInput = document.getElementById('stopapp');
    const applicationName = appNameInput.value;

    if (ws.readyState === WebSocket.OPEN){
        ws.send("stop_app:" + applicationName);
        appNameInput.value = ""
    }
    else {
        log("WebSocket chưa kết nối xong.");
    }
}
let stopApp = document.getElementById("StopApp");
stopApp.addEventListener('submit', StopApp)

function screenShot(){
    if (ws.readyState === WebSocket.OPEN){
        ws.send("screenshot");
    }
    else {
        log("WebSocket chưa kết nối xong.");
    }
}