void StartProcName(const std::string& procName, bool& flag, DWORD& pid) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Chuẩn hóa tên file có đuôi .exe
    std::wstring wProcName = ToWString(procName);
    if (wProcName.size() < 4 || wProcName.substr(wProcName.size() - 4) != L".exe") {
        wProcName += L".exe";
    }

    wchar_t targetPath[MAX_PATH] = { 0 };

    // --- TẦNG 1: Dùng SearchPathW (Tìm trong System32, Windows, PATH) ---
    DWORD len = SearchPathW(nullptr, wProcName.c_str(), nullptr, MAX_PATH, targetPath, nullptr);

    // --- TẦNG 2: Nếu Tầng 1 thất bại, kiểm tra xem procName có phải đường dẫn tuyệt đối trực tiếp ---
    if (len == 0) {
        DWORD attr = GetFileAttributesW(wProcName.c_str());
        if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            wcscpy_s(targetPath, MAX_PATH, wProcName.c_str());
            len = (DWORD)wProcName.length();
        }
    }

    // --- TẦNG 3: Nếu vẫn chưa thấy, quét sâu trong Program Files và ổ đĩa ---
    if (len == 0) {
        std::wstring foundPath = L"";
        // Giới hạn quét Program Files để tăng tốc độ phản hồi
        if (FindFileRecursive(L"C:\\Program Files", wProcName, foundPath) ||
            FindFileRecursive(L"C:\\Program Files (x86)", wProcName, foundPath) ||
            FindFileRecursive(L"D:\\", wProcName, foundPath)) 
        {
            wcscpy_s(targetPath, MAX_PATH, foundPath.c_str());
            len = (DWORD)foundPath.length();
        }
    }

    // --- THỰC THI TIẾN TRÌNH ---
    if (len > 0) {
        if (CreateProcessW(targetPath, nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            pid = pi.dwProcessId;
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            flag = true;
            std::wcout << L"Successfully started: " << targetPath << L" (PID: " << pid << L")" << std::endl;
            return;
        }
    }

    flag = false;
    pid = 0;
}