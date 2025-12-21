#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include "ConvertString.cpp"

bool StopProcessByName(const std::wstring& processName) {
    bool killed = false;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(hSnapshot, &pe))
    {
        do
        {
            if (_wcsicmp(pe.szExeFile, processName.c_str()) == 0)
            {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess)
                {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                    killed = true;
                }
            }
        } while (Process32NextW(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return killed;
}

void StopApplication(const std::string& appName, bool &flag) {
	std::wstring wAppName = ToWString(appName);
    if (wAppName.substr(wAppName.length() - 4) != L".exe") {
        wAppName += L".exe";
	}
    flag = StopProcessByName(wAppName);
}