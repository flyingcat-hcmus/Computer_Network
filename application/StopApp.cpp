#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>

bool StopProcessByName(const std::wstring& processName) {
    bool killedAtLeastOne = false; // Biến cờ để theo dõi xem có xóa được cái nào không
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            // So sánh tên process (không phân biệt hoa thường)
            if (_wcsicmp(pe.szExeFile, processName.c_str()) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                    
                    // Đánh dấu là đã xóa được ít nhất 1 process
                    killedAtLeastOne = true; 
                    
                    // QUAN TRỌNG: Không đóng hSnapshot ở đây và không return ở đây
                    // Để vòng lặp tiếp tục chạy và tìm các PID khác có cùng tên
                }
            }
        } while (Process32NextW(hSnapshot, &pe));
    }

    // Chỉ đóng snapshot sau khi đã duyệt hết toàn bộ danh sách
    CloseHandle(hSnapshot);
    
    return killedAtLeastOne; // Trả về true nếu xóa được ít nhất 1 cái, false nếu không tìm thấy cái nào
}

void StopApplication(const std::string& appName, bool &flag) {
    // Lưu ý: Cách convert này chỉ đúng với tiếng Anh không dấu. 
    // Nếu appName có tiếng Việt hoặc ký tự đặc biệt, cần dùng MultiByteToWideChar.
    std::wstring wAppName(appName.begin(), appName.end());
    flag = StopProcessByName(wAppName);
}