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
    
    bool flag = 0;
    if (Process32FirstW(hSnapshot, &pe)) {
        bool flag1 = 0;
        do {
            flag1 = 0;
            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hSnapshot == INVALID_HANDLE_VALUE)
                return false;

            PROCESSENTRY32W pe;
            pe.dwSize = sizeof(PROCESSENTRY32W);
            if (Process32FirstW(hSnapshot, &pe)) {
                do {
                    if (_wcsicmp(pe.szExeFile, processName.c_str()) == 0) {
                        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                        if (hProcess) {
                            TerminateProcess(hProcess, 0);
                            CloseHandle(hProcess);
                            CloseHandle(hSnapshot);
                            flag1 = flag = 1;
                            //return true;    // Stopped
                        }
                    }
                } while (Process32NextW(hSnapshot, &pe));
            }
        } while (flag1);
    }

    if (flag) CloseHandle(hSnapshot);
    return flag;   // Not found
}

void StopApplication(const std::string& appName, bool &flag) {
    // Lưu ý: Cách convert này chỉ đúng với tiếng Anh không dấu. 
    // Nếu appName có tiếng Việt hoặc ký tự đặc biệt, cần dùng MultiByteToWideChar.
    std::wstring wAppName(appName.begin(), appName.end());
    flag = StopProcessByName(wAppName);
}