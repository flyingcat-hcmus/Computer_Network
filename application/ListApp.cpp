#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "ConvertString.cpp"

// Link thư viện Registry
#pragma comment(lib, "advapi32.lib")

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// --- HÀM HỖ TRỢ CHUYỂN ĐỔI UNICODE SANG UTF-8 (Để gửi qua WebSocket) ---

// --- HÀM ĐỌC REGISTRY TỪ MỘT ĐƯỜNG DẪN CỤ THỂ ---
void GetAppsFromKey(HKEY hKeyRoot, LPCWSTR path, std::vector<std::string>& appList) {
    HKEY hKey;
    // Mở khóa Registry chính
    if (RegOpenKeyExW(hKeyRoot, path, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return;
    }

    WCHAR achKey[MAX_KEY_LENGTH];   // Tên subkey (GUID)
    DWORD cbName;                   // Kích thước tên
    DWORD cSubKeys = 0;             // Số lượng phần mềm

    // Lấy thông tin số lượng subkey
    RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    if (cSubKeys > 0) {
        for (DWORD i = 0; i < cSubKeys; i++) {
            cbName = MAX_KEY_LENGTH;
            // Duyệt qua từng GUID phần mềm
            if (RegEnumKeyExW(hKey, i, achKey, &cbName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                
                HKEY hSubKey;
                // Mở subkey chi tiết
                if (RegOpenKeyExW(hKey, achKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    
                    WCHAR szName[MAX_VALUE_NAME];
                    DWORD dwSize = sizeof(szName);
                    DWORD dwType;
		    // Đọc giá trị "DisplayName"
                    if (RegQueryValueExW(hSubKey, L"DisplayName", NULL, &dwType, (LPBYTE)szName, &dwSize) == ERROR_SUCCESS) {
                        std::wstring wName(szName);
                        
                        // Lọc bớt các bản cập nhật hệ thống (System Component) nếu muốn danh sách gọn hơn
                        // DWORD isSystemComponent = 0;
                        // DWORD dwSizeSys = sizeof(isSystemComponent);
                        // RegQueryValueExW(hSubKey, L"SystemComponent", NULL, NULL, (LPBYTE)&isSystemComponent, &dwSizeSys);
                        
                        if (!wName.empty()) {
                            appList.push_back(ToUtf8(wName));
                        }
                    }   
                    RegCloseKey(hSubKey);
                }
            }
        }
    }
    RegCloseKey(hKey);
}

// --- HÀM CHÍNH ĐỂ GỌI TỪ MAIN/WEBSOCKET ---
void ListApplication(std::string& ans) {
    std::vector<std::string> apps;

    // 1. Quét phần mềm 64-bit (Native)
    GetAppsFromKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);

    // 2. Quét phần mềm 32-bit trên Win 64-bit (WOW6432Node) - Rất quan trọng vì Chrome, Steam thường nằm đây
    GetAppsFromKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);

    // 3. Quét phần mềm User hiện tại (Current User) - Ví dụ: VS Code, Zoom
    GetAppsFromKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall", apps);

    // Nối tất cả thành 1 chuỗi, ngăn cách bởi xuống dòng
    std::string result = "";
    for (const auto& app : apps) {
        result += app + "\n";
    }
    ans = result;
}