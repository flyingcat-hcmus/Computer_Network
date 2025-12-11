let ws = new WebSocket("ws://192.168.1.7:9000"); // IP server
ws.binaryType = "arraybuffer";

// Nếu flag = 0 thì đây là msg bth
// flag = 1 thì screenshot
// flag = 2 thì webcam
let flag = -1

ws.onopen = () => {
    log("Connected!");
    
};

// // Mỗi khi server gửi lại client bằng s->send thì
// // data được truyền vào "e"
ws.onmessage = (event) => {

    if (event.data instanceof ArrayBuffer) {
        
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
    } 
    else {
        console.log("--> Là tin nhắn văn bản:", event.data);
    }

    // 2. Tạo Blob từ ArrayBuffer nhận được
    const blob = new Blob([event.data], { type: 'video/mp4' });

    // 3. Tạo URL ảo
    const videoUrl = URL.createObjectURL(blob);

    // 4. Gán vào Video Player
    const videoPlayer = document.getElementById('videoPlayer');
            
    // Xóa URL cũ để giải phóng bộ nhớ (nếu có)
    if (videoPlayer.src) {
        URL.revokeObjectURL(videoPlayer.src);
    }

    videoPlayer.src = videoUrl;
            
    videoPlayer.play()
        .then(() => console.log("Đang phát video"))
        .catch(e => console.error("Lỗi Autoplay:", e));

    document.getElementById('status').innerText = "Đang phát video.";
}

function HandleClientMSG(data){
    if (data == "screenshot"){
        flag = 1;
    }
    else if (data == "webcam"){
        flag = 2;
    }
    else flag = 0
}

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

function webCam(){
    if (ws.readyState === WebSocket.OPEN){
        ws.send("webcam");
    }
    else {
        log("WebSocket chưa kết nối xong.");
    }
}