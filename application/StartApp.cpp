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

bool StartApplication(const std::string& appName) {
    std::wstring wAppName = ToWString(appName);
    std::wstring foundPath = L"";

    // Tìm file trong ổ D
    if (FindFileRecursive(L"D:\\", wAppName + L".exe", foundPath)) {
        // Khởi động ứng dụng
        ShellExecuteW(NULL, L"open", foundPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        return true;
    }
    // Tìm file trong ổ C
    else if (FindFileRecursive(L"C:\\", wAppName + L".exe", foundPath)) {
        // Khởi động ứng dụng
        ShellExecuteW(NULL, L"open", foundPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        return true;
    }
    else {
        return false;
    }
}