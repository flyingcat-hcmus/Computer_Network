#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <io.h>
#include <string>
#include "ConvertString.cpp"

// Link thư viện Registry
#pragma comment(lib, "advapi32.lib")

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// Hàm đọc và in danh sách phần mềm từ một đường dẫn Registry cụ thể
std::wstring ListApplicationFromPath(HKEY hKeyRoot, LPCWSTR path) {
    std::wstring s; // Luu danh sách phần mềm
    HKEY hKey;
    // Mở khóa Registry chính (Uninstall)
    if (RegOpenKeyExW(hKeyRoot, path, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return L"";
    }

    WCHAR    achKey[MAX_KEY_LENGTH];   // Buffer chứa tên subkey
    DWORD    cbName;                   // Kích thước tên subkey
    DWORD    cSubKeys = 0;             // Số lượng subkey

    // Lấy thông tin về số lượng phần mềm (subkeys)
    RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    if (cSubKeys > 0) {
        // Duyệt qua từng phần mềm
        for (DWORD i = 0; i < cSubKeys; i++) {
            cbName = MAX_KEY_LENGTH;
            // Lấy tên định danh của phần mềm (thường là chuỗi GUID loằng ngoằng)
            if (RegEnumKeyExW(hKey, i, achKey, &cbName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                
                // Mở subkey của phần mềm đó để đọc chi tiết
                HKEY hSubKey;
                if (RegOpenKeyExW(hKey, achKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    
                    WCHAR szName[MAX_VALUE_NAME];
                    DWORD dwSize = sizeof(szName);
                    DWORD dwType;

                    // Đọc giá trị "DisplayName" (Tên hiển thị của phần mềm)
                    if (RegQueryValueExW(hSubKey, L"DisplayName", NULL, &dwType, (LPBYTE)szName, &dwSize) == ERROR_SUCCESS) {
                        // In ra màn hình (dùng wcout để in tiếng Việt/Unicode)
                        s += std::wstring(szName) + L"\n";
                    }   
                    RegCloseKey(hSubKey);
                }
            }
        }
    }
    RegCloseKey(hKey);
    return s;
}

std::string ListApplication() {
    // Chế độ in Unicode ra console (để không bị lỗi font tên phần mềm)
    int oldMode = _setmode(_fileno(stdout), _O_WTEXT); // was _O_U16TEXT
    _setmode(_fileno(stdout), _O_U16TEXT);
    std::wstring s;

    // std::wcout << L"=== DANH SACH PHAN MEM DA CAI DAT (64-bit) ===" << std::endl;
    // 1. Quét phần mềm 64-bit native
    s += ListApplicationFromPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");

    // std::wcout << L"\n=== DANH SACH PHAN MEM DA CAI DAT (32-bit) ===" << std::endl;
    // 2. Quét phần mềm 32-bit (nằm trong WOW6432Node)
    s += ListApplicationFromPath(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    
    // 3. Quét phần mềm cài riêng cho User hiện tại (ít gặp hơn nhưng vẫn có)
    // std::wcout << L"\n=== PHAN MEM CUA USER HIEN TAI ===" << std::endl;
    s += ListApplicationFromPath(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");

    _setmode(_fileno(stdout), oldMode);
    
    return ToUtf8(s);
}