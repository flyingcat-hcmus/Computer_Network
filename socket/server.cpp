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

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "../Application/ListApp.cpp"  // Chèn hàm liệt kê ứng dụng đã cài đặt
#include "../Application/StartApp.cpp" // Chèn hàm khởi động ứng dụng
#include "../Application/StopApp.cpp"  // Chèn hàm tắt ứng dụng

#include "../Screen Shot/ScreenShot.cpp" // Chèn hàm chụp màn hình

#include <iostream>
#include <string>

typedef websocketpp::server<websocketpp::config::asio> server;

void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg) {

    std::string received = msg->get_payload();
    std::cout << "Received: " << received << std::endl;

    if (received == "list_apps") {
        // Gọi hàm liệt kê ứng dụng và gửi kết quả về client
        std::string app_list = ListApplication(); // Giả sử hàm này trả về danh sách ứng dụng đã cài đặt
        s->send(hdl, app_list, msg->get_opcode());
    }

    if (received.rfind("start_app:", 0) == 0) {
        std::string app_to_start = received.substr(10); // Lấy tên ứng dụng sau "start_app:"
        if (!StartApplication(app_to_start)) {
            std::cout << "Failed to start application: " << app_to_start << std::endl;
            s->send(hdl, "Failed to start application: " + app_to_start, msg->get_opcode());
            return;
        } 
        s->send(hdl, "Starting application: " + app_to_start, msg->get_opcode());
    }

    if (received.rfind("stop_app:", 0) == 0) {
        std::string app_to_stop = received.substr(9); // Lấy tên ứng dụng sau "stop_app:"
        if (!StopApplication(app_to_stop)) {
            std::cout << "Failed to stop application: " << app_to_stop << std::endl;
            s->send(hdl, "Failed to stop application: " + app_to_stop, msg->get_opcode());
            return;
        } 
        s->send(hdl, "Stopping application: " + app_to_stop, msg->get_opcode());
    }

    if (received == "screenshot") {
        // 1. Chụp màn hình và lưu vào file
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
    }
}

int main() {

    //----> COMPILE = 
    // g++ -std=c++17 -I./ -I./asio/include server.cpp -o server.exe -lmswsock  -lws2_32 -lgdi32 -luser32
    
    server s;

    try {
        // comment 2 dong nay roi chay List App se ra chu TRUNG QUOC
        s.clear_access_channels(websocketpp::log::alevel::all);
        s.clear_error_channels(websocketpp::log::elevel::all);

        s.init_asio();
        s.set_message_handler(std::bind(&on_message, &s, std::placeholders::_1, std::placeholders::_2));

        s.listen(9000);          // Cổng server
        s.start_accept();

        std::cout << "Server running at ws://localhost:9000\n";
        s.run();  // chạy event loop
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
