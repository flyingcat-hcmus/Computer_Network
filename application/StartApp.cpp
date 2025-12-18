#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <string>
#include <direct.h>
#include <filesystem>
#include "ConvertString.cpp"

bool StartAppViaAppPaths(const std::wstring& exeName)
{
    HKEY hKey;
    std::wstring keyPath =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\" + exeName;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS &&
        RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return false;

    WCHAR path[MAX_PATH];
    DWORD size = sizeof(path);

    if (RegQueryValueExW(hKey, nullptr, nullptr, nullptr, (LPBYTE)path, &size) == ERROR_SUCCESS)
    {
        ShellExecuteW(nullptr, L"open", path, nullptr, nullptr, SW_SHOWNORMAL);
        RegCloseKey(hKey);
        return true;
    }

    RegCloseKey(hKey);
    return false;
}

void StartApplication(const std::string& appName, bool& flag)
{
    std::wstring wApp = ToWString(appName);

    // 1. Thử để Windows tự tìm
    HINSTANCE h = ShellExecuteW(nullptr, L"open", wApp.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

    if ((INT_PTR)h > 32) {
        flag = true;
        return;
    }

    // 2. Thử App Paths
    if (StartAppViaAppPaths(wApp + L".exe")) {
        flag = true;
        return;
    }

    flag = false;
}

/*
// Cách đơn giản hơn nhưng không bao quát bằng cách trên
bool StartApp(const std::wstring& name)
{
    // 1. Để Windows tự resolve
    HINSTANCE h = ShellExecuteW(nullptr, L"open", name.c_str(),
                                nullptr, nullptr, SW_SHOWNORMAL);
    if ((INT_PTR)h > 32)
        return true;

    // 2. Thử name.exe
    h = ShellExecuteW(nullptr, L"open", (name + L".exe").c_str(),
                      nullptr, nullptr, SW_SHOWNORMAL);
    return (INT_PTR)h > 32;
} 
*/