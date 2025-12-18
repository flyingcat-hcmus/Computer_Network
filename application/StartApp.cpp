#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <string>
#include <direct.h>
#include <filesystem>
#include "ConvertString.cpp"

// Tìm file theo tên trong toàn bộ ổ D
bool FindFileRecursive(const std::wstring& folder, const std::wstring& targetName, std::wstring& outputPath)
{

    WIN32_FIND_DATAW fd;
    HANDLE hFind;

    std::wstring search = folder + L"\\*";

    hFind = FindFirstFileW(search.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;

    do {
        std::wstring name = fd.cFileName;

        // Skip . ..
        if (name == L"." || name == L"..")
            continue;

        std::wstring fullPath = folder + L"\\" + name;

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            // Đệ quy vào thư mục con
            if (FindFileRecursive(fullPath, targetName, outputPath))
            {
                FindClose(hFind);
                return true;
            }
        }
        else
        {
            // Kiểm tra tên file
            if (_wcsicmp(name.c_str(), targetName.c_str()) == 0)
            {
                outputPath = fullPath;
                FindClose(hFind);
                return true;
            }
        }

    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
    return false;
}

void StartApplication(const std::string& appName, bool& flag) {
    std::wstring wAppName = ToWString(appName);
    std::wstring foundPath = L"";

    // --- TRƯỜNG HỢP 1: appName LÀ ĐƯỜNG DẪN CỤ THỂ ---
    // Kiểm tra xem chuỗi đầu vào có phải là đường dẫn file tồn tại không
    DWORD fileAttr = GetFileAttributesW(wAppName.c_str());

    // Nếu file tồn tại (không lỗi) VÀ không phải là thư mục
    if (fileAttr != INVALID_FILE_ATTRIBUTES && !(fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        ShellExecuteW(NULL, L"open", wAppName.c_str(), NULL, NULL, SW_SHOWNORMAL);
        flag = true;
        return; // Chạy xong thì thoát luôn, không cần tìm kiếm nữa
    }

    // --- TRƯỜNG HỢP 2: appName LÀ TÊN APP -> TÌM KIẾM ĐỆ QUY ---
    // (Logic cũ của bạn)

    // Tìm file trong ổ D
    if (FindFileRecursive(L"D:\\", wAppName, foundPath)) {
        ShellExecuteW(NULL, L"open", foundPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        flag = true;
    }
    // Tìm file trong ổ C
    else if (FindFileRecursive(L"C:\\", wAppName, foundPath)) {
        ShellExecuteW(NULL, L"open", foundPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        flag = true;
    }
    else {
        flag = false;
    }
}