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
//void GetAppsFromKey(HKEY hKeyRoot, LPCWSTR path, std::vector<std::string>& appList) {
//    HKEY hKey;
//    // Mở khóa Registry chính
//    if (RegOpenKeyExW(hKeyRoot, path, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
//        return;
//    }
//
//    WCHAR achKey[MAX_KEY_LENGTH];   // Tên subkey (GUID)
//    DWORD cbName;                   // Kích thước tên
//    DWORD cSubKeys = 0;             // Số lượng phần mềm
//
//    // Lấy thông tin số lượng subkey
//    RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
//
//    if (cSubKeys > 0) {
//        for (DWORD i = 0; i < cSubKeys; i++) {
//            cbName = MAX_KEY_LENGTH;
//            // Duyệt qua từng GUID phần mềm
//            if (RegEnumKeyExW(hKey, i, achKey, &cbName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
//                
//                HKEY hSubKey;
//                // Mở subkey chi tiết
//                if (RegOpenKeyExW(hKey, achKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
//                    
//                    WCHAR szName[MAX_VALUE_NAME];
//                    DWORD dwSize = sizeof(szName);
//                    DWORD dwType;
//		    // Đọc giá trị "DisplayName"
//                    if (RegQueryValueExW(hSubKey, L"DisplayName", NULL, &dwType, (LPBYTE)szName, &dwSize) == ERROR_SUCCESS) {
//                        std::wstring wName(szName);
//                        
//                        // Lọc bớt các bản cập nhật hệ thống (System Component) nếu muốn danh sách gọn hơn
//                        // DWORD isSystemComponent = 0;
//                        // DWORD dwSizeSys = sizeof(isSystemComponent);
//                        // RegQueryValueExW(hSubKey, L"SystemComponent", NULL, NULL, (LPBYTE)&isSystemComponent, &dwSizeSys);
//                        
//                        if (!wName.empty()) {
//                            appList.push_back(ToUtf8(wName));
//                        }
//                    }   
//                    RegCloseKey(hSubKey);
//                }
//            }
//        }
//    }
//    RegCloseKey(hKey);
//}

//void GetAppsFromKey(HKEY hKeyRoot, LPCWSTR path, std::vector<std::string>& appList) {
//    HKEY hKey;
//    // Mở khóa Registry chính
//    if (RegOpenKeyExW(hKeyRoot, path, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
//        return;
//    }
//
//    WCHAR achKey[MAX_KEY_LENGTH];   // Tên subkey (GUID)
//    DWORD cbName;                   // Kích thước tên
//    DWORD cSubKeys = 0;             // Số lượng phần mềm
//
//    // Lấy thông tin số lượng subkey
//    RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
//
//    if (cSubKeys > 0) {
//        for (DWORD i = 0; i < cSubKeys; i++) {
//            cbName = MAX_KEY_LENGTH;
//            // Duyệt qua từng GUID phần mềm
//            if (RegEnumKeyExW(hKey, i, achKey, &cbName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
//
//                HKEY hSubKey;
//                // Mở subkey chi tiết
//                if (RegOpenKeyExW(hKey, achKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
//
//                    WCHAR szName[MAX_VALUE_NAME] = { 0 };
//                    WCHAR szPath[MAX_PATH] = { 0 }; // Biến lưu đường dẫn
//
//                    DWORD dwSizeName = sizeof(szName);
//                    DWORD dwSizePath = sizeof(szPath);
//                    DWORD dwType;
//
//                    // 1. Đọc giá trị "DisplayName" (Tên phần mềm)
//                    if (RegQueryValueExW(hSubKey, L"DisplayName", NULL, &dwType, (LPBYTE)szName, &dwSizeName) == ERROR_SUCCESS) {
//                        std::wstring wName(szName);
//                        std::wstring wPath;
//
//                        // 2. Cố gắng đọc "InstallLocation" (Thư mục cài đặt)
//                        dwSizePath = sizeof(szPath); // Reset size
//                        if (RegQueryValueExW(hSubKey, L"InstallLocation", NULL, NULL, (LPBYTE)szPath, &dwSizePath) == ERROR_SUCCESS) {
//                            wPath = szPath;
//                        }
//                        // 3. Nếu không có InstallLocation, thử đọc "DisplayIcon" (Thường trỏ thẳng đến file .exe)
//                        else {
//                            dwSizePath = sizeof(szPath); // Reset size
//                            if (RegQueryValueExW(hSubKey, L"DisplayIcon", NULL, NULL, (LPBYTE)szPath, &dwSizePath) == ERROR_SUCCESS) {
//                                // DisplayIcon thường có dạng "C:\Program Files\App\app.exe,0" -> Cần cắt bỏ ",0"
//                                wPath = szPath;
//                                size_t commaPos = wPath.find(L',');
//                                if (commaPos != std::wstring::npos) {
//                                    wPath = wPath.substr(0, commaPos);
//                                }
//                            }
//                            else {
//                                wPath = L"Unknown Path"; // Không tìm thấy đường dẫn
//                            }
//                        }
//
//                        if (!wName.empty()) {
//                            // Format dữ liệu trả về để dễ parse: "Name: <tên> | Path: <đường dẫn>"
//                            std::string entry = "Name: " + ToUtf8(wName) + " | Path: " + ToUtf8(wPath);
//                            appList.push_back(entry);
//                        }
//                    }
//                    RegCloseKey(hSubKey);
//                }
//            }
//        }
//    }
//    RegCloseKey(hKey);
//}

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm> // Để dùng std::replace

// Hàm phụ trợ: Làm sạch đường dẫn (Xóa ngoặc kép, xóa tham số sau dấu phẩy)
std::wstring CleanPath(std::wstring path) {
    if (path.empty()) return L"";

    // 1. Xóa dấu ngoặc kép bao quanh (nếu có)
    if (path.front() == L'"' && path.back() == L'"') {
        path = path.substr(1, path.length() - 2);
    }
    // Hoặc xóa chỉ dấu ngoặc kép đầu/cuối nếu nó bị sót
    path.erase(std::remove(path.begin(), path.end(), L'"'), path.end());

    // 2. Xóa các tham số icon (VD: "C:\App.exe,0" -> cắt bỏ từ dấu phẩy)
    size_t commaPos = path.find(L',');
    if (commaPos != std::wstring::npos) {
        path = path.substr(0, commaPos);
    }

    // 3. Trim khoảng trắng thừa (nếu cần - đơn giản hóa ở đây)
    return path;
}

// Hàm kiểm tra xem đường dẫn có kết thúc bằng .exe không
bool IsExeFile(const std::wstring& path) {
    if (path.length() < 4) return false;
    std::wstring ext = path.substr(path.length() - 4);
    // Chuyển về chữ thường để so sánh
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == L".exe";
}

void GetAppsFromKey(HKEY hKeyRoot, LPCWSTR path, std::vector<std::string>& appList) {
    HKEY hKey;
    if (RegOpenKeyExW(hKeyRoot, path, 0, KEY_READ, &hKey) != ERROR_SUCCESS) return;

    WCHAR achKey[MAX_KEY_LENGTH];
    DWORD cbName;
    DWORD cSubKeys = 0;

    RegQueryInfoKeyW(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    if (cSubKeys > 0) {
        for (DWORD i = 0; i < cSubKeys; i++) {
            cbName = MAX_KEY_LENGTH;
            if (RegEnumKeyExW(hKey, i, achKey, &cbName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

                HKEY hSubKey;
                if (RegOpenKeyExW(hKey, achKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {

                    WCHAR szName[MAX_VALUE_NAME] = { 0 };
                    WCHAR szPathBuf[MAX_PATH] = { 0 };
                    DWORD dwSizeName = sizeof(szName);
                    DWORD dwSizePath = sizeof(szPathBuf);
                    DWORD dwType;

                    // Lấy Tên
                    if (RegQueryValueExW(hSubKey, L"DisplayName", NULL, &dwType, (LPBYTE)szName, &dwSizeName) == ERROR_SUCCESS) {
                        std::wstring wName(szName);
                        std::wstring finalPath = L"";

                        // CHIẾN THUẬT TÌM FILE EXE:

                        // Ưu tiên 1: DisplayIcon (Thường chứa đường dẫn file .exe)
                        dwSizePath = sizeof(szPathBuf);
                        if (RegQueryValueExW(hSubKey, L"DisplayIcon", NULL, NULL, (LPBYTE)szPathBuf, &dwSizePath) == ERROR_SUCCESS) {
                            std::wstring iconPath = CleanPath(szPathBuf);
                            if (IsExeFile(iconPath)) {
                                finalPath = iconPath;
                            }
                        }

                        // Ưu tiên 2: Nếu DisplayIcon thất bại, thử InstallLocation
                        //if (finalPath.empty()) {
                        //    dwSizePath = sizeof(szPathBuf);
                        //    if (RegQueryValueExW(hSubKey, L"InstallLocation", NULL, NULL, (LPBYTE)szPathBuf, &dwSizePath) == ERROR_SUCCESS) {
                        //        std::wstring installLoc = CleanPath(szPathBuf);
                        //        // InstallLocation thường là Folder, nên ta chấp nhận nó dù không phải .exe
                        //        finalPath = installLoc;
                        //    }
                        //}

                        // Ưu tiên 3: UninstallString (Thường trỏ đến uninstall.exe, nhưng nó nằm cùng thư mục với app chính)
                        //if (finalPath.empty()) {
                        //    dwSizePath = sizeof(szPathBuf);
                        //    if (RegQueryValueExW(hSubKey, L"UninstallString", NULL, NULL, (LPBYTE)szPathBuf, &dwSizePath) == ERROR_SUCCESS) {
                        //        finalPath = CleanPath(szPathBuf); // Cái này chắc chắn là file exe, nhưng là file gỡ cài đặt
                        //    }
                        //}

                        if (!wName.empty()) {
                            std::string entry = ToUtf8(wName);
                            if (!finalPath.empty()) {
                                entry += "|" + ToUtf8(finalPath);
                            }
                            else {
                                entry += "|Not Found";
                            }
                            appList.push_back(entry);
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