#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <iomanip> // For formatting output

void ListRunningProcesses() {
    // 1. Take a snapshot of all processes in the system.
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: CreateToolhelp32Snapshot failed." << std::endl;
        return;
    }

    // 2. Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // 3. Retrieve information about the first process.
    if (!Process32First(hProcessSnap, &pe32)) {
        std::cerr << "Error: Process32First failed." << std::endl;
        CloseHandle(hProcessSnap);
        return;
    }

    // Header formatting
    std::cout << std::left << std::setw(10) << "PID"
              << std::setw(30) << "Process Name"
              << "Threads" << std::endl;
    std::cout << "--------------------------------------------------------" << std::endl;

    // 4. Walk the snapshot of processes using do-while loop
    do {
        std::wcout << std::left << std::setw(10) << pe32.th32ProcessID
                   << std::setw(30) << pe32.szExeFile
                   << pe32.cntThreads << std::endl;

    } while (Process32Next(hProcessSnap, &pe32));

    // 5. Clean up the handle
    CloseHandle(hProcessSnap);
}

int main() {
    ListRunningProcesses();
    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    return 0;
}
