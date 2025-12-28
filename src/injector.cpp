#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>

DWORD GetProcessId(const wchar_t* processName) {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32W pe = { sizeof(pe) };
    while (Process32NextW(snap, &pe)) {
        if (_wcsicmp(pe.szExeFile, processName) == 0) {
            pid = pe.th32ProcessID;
            break;
        }
    }

    CloseHandle(snap);
    return pid;
}

bool InjectDLL(DWORD pid, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        printf("[!] Failed to open process (Error: %lu)\n", GetLastError());
        return false;
    }

    size_t pathLen = strlen(dllPath) + 1;
    LPVOID remotePath = VirtualAllocEx(hProcess, nullptr, pathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remotePath) {
        printf("[!] Failed to allocate memory (Error: %lu)\n", GetLastError());
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, remotePath, dllPath, pathLen, nullptr)) {
        printf("[!] Failed to write memory (Error: %lu)\n", GetLastError());
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    LPVOID loadLibrary = GetProcAddress(kernel32, "LoadLibraryA");

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
        (LPTHREAD_START_ROUTINE)loadLibrary, remotePath, 0, nullptr);

    if (!hThread) {
        printf("[!] Failed to create remote thread (Error: %lu)\n", GetLastError());
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return exitCode != 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Get DLL path (same folder as injector)
    char dllPath[MAX_PATH];
    GetModuleFileNameA(nullptr, dllPath, MAX_PATH);

    char* lastSlash = strrchr(dllPath, '\\');
    if (lastSlash) {
        strcpy(lastSlash + 1, "InternalAimbot.dll");
    }

    // Check if DLL exists
    if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxA(nullptr, "InternalAimbot.dll not found!", "GFR Injector", MB_ICONERROR);
        return 1;
    }

    // Wait for game
    DWORD pid = 0;
    while (pid == 0) {
        pid = GetProcessId(L"Gunfire Reborn.exe");
        if (pid == 0) Sleep(1000);
    }

    // Wait a bit for game to initialize
    Sleep(2000);

    if (InjectDLL(pid, dllPath)) {
        // Success - silent
    } else {
        MessageBoxA(nullptr, "Injection failed!", "GFR Injector", MB_ICONERROR);
    }

    return 0;
}
