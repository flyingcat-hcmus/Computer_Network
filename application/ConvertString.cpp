#pragma once
#include <windows.h>
#include <string>

// Hàm chuyển đổi wstring sang UTF-8 chuẩn
std::string ToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Hàm chuyển đổi từ std::string (UTF-8) sang std::wstring (Unicode/UTF-16)
std::wstring ToWString(const std::string& str) {
    // Nếu chuỗi rỗng thì trả về rỗng ngay
    if (str.empty()) return std::wstring();

    // Bước 1: Tính toán độ dài bộ nhớ cần thiết cho chuỗi đích
    // CP_UTF8: Bảo hệ thống là nguồn vào đang là UTF-8
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    
    // Bước 2: Cấp phát chuỗi wstring với độ dài vừa tính
    std::wstring wstrTo(size_needed, 0);
    
    // Bước 3: Thực hiện chuyển đổi thật
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    
    return wstrTo;
}