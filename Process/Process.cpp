#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <iomanip> // For formatting output
#include "../Application/ConvertString.cpp"

void ListRunningProcesses(std::string& ans) {
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32; // Sử dụng bản Unicode tường minh

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        ans = "";
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        ans = "";
        return;
    }

    std::wstring s = L""; 
    
    do {
        // TRỰC TIẾP khởi tạo wstring từ WCHAR array (szExeFile)
        // Cách này không gây lỗi constructor vì kiểu dữ liệu tương thích hoàn toàn
        std::wstring wExeName = pe32.szExeFile; 

        s += L"PID: " + std::to_wstring(pe32.th32ProcessID) + 
             L", Name: " + wExeName + 
             L", Threads: " + std::to_wstring(pe32.cntThreads) + L"\n";

    } while (Process32NextW(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    
    // Sử dụng hàm ToUtf8 bạn đã include để chuyển từ wstring sang string (UTF-8) để gửi qua socket
    ans = ToUtf8(s);
}