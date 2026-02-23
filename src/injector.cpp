#include <Windows.h>
#include <TlHelp32.h>
#include <WinHttp.h>
#include <urlmon.h>
#include <cstdio>
#include <cctype>
#include <fstream>
#include <string>
#include <vector>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "urlmon.lib")

#ifndef GFR_MOD_VERSION
#define GFR_MOD_VERSION "v0.0.0-dev"
#endif

#ifndef GFR_GITHUB_OWNER
#define GFR_GITHUB_OWNER ""
#endif

#ifndef GFR_GITHUB_REPO
#define GFR_GITHUB_REPO ""
#endif

#ifndef GFR_GITHUB_ASSET
#define GFR_GITHUB_ASSET "InternalAimbot.dll"
#endif

namespace {

void DebugLog(const char* message) {
    if (!message) return;
    OutputDebugStringA(message);
}

bool FileExists(const std::string& path) {
    DWORD attrs = GetFileAttributesA(path.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

std::string TrimCopy(const std::string& text) {
    size_t begin = 0;
    while (begin < text.size() && std::isspace((unsigned char)text[begin])) begin++;
    size_t end = text.size();
    while (end > begin && std::isspace((unsigned char)text[end - 1])) end--;
    return text.substr(begin, end - begin);
}

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring out((size_t)len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, out.data(), len);
    return out;
}

bool ParseJsonStringAfterKey(const std::string& json, size_t keyPos, std::string* outValue) {
    if (!outValue || keyPos == std::string::npos) return false;

    size_t colon = json.find(':', keyPos);
    if (colon == std::string::npos) return false;

    size_t i = colon + 1;
    while (i < json.size()) {
        char c = json[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            i++;
            continue;
        }
        break;
    }

    if (i >= json.size() || json[i] != '"') return false;
    i++;

    std::string value;
    bool escaped = false;
    for (; i < json.size(); i++) {
        char c = json[i];
        if (escaped) {
            switch (c) {
            case '"': value.push_back('"'); break;
            case '\\': value.push_back('\\'); break;
            case '/': value.push_back('/'); break;
            case 'b': value.push_back('\b'); break;
            case 'f': value.push_back('\f'); break;
            case 'n': value.push_back('\n'); break;
            case 'r': value.push_back('\r'); break;
            case 't': value.push_back('\t'); break;
            default: value.push_back(c); break;
            }
            escaped = false;
            continue;
        }

        if (c == '\\') {
            escaped = true;
            continue;
        }
        if (c == '"') {
            *outValue = value;
            return true;
        }
        value.push_back(c);
    }

    return false;
}

std::string JsonGetString(const std::string& json, const char* key, size_t startPos = 0) {
    if (!key || !key[0]) return "";
    std::string token = "\"";
    token += key;
    token += "\"";

    size_t keyPos = json.find(token, startPos);
    if (keyPos == std::string::npos) return "";

    std::string value;
    if (!ParseJsonStringAfterKey(json, keyPos, &value)) return "";
    return value;
}

bool EndsWithInsensitive(const std::string& value, const char* suffix) {
    if (!suffix) return false;
    std::string sfx = suffix;
    if (value.size() < sfx.size()) return false;

    size_t offset = value.size() - sfx.size();
    for (size_t i = 0; i < sfx.size(); i++) {
        char a = (char)std::tolower((unsigned char)value[offset + i]);
        char b = (char)std::tolower((unsigned char)sfx[i]);
        if (a != b) return false;
    }
    return true;
}

std::string FindAssetDownloadUrl(const std::string& json, const std::string& wantedAssetName) {
    size_t searchPos = 0;
    while (true) {
        std::string nameToken = "\"name\"";
        size_t namePos = json.find(nameToken, searchPos);
        if (namePos == std::string::npos) break;

        std::string name;
        if (ParseJsonStringAfterKey(json, namePos, &name)) {
            bool nameMatched = false;
            if (!wantedAssetName.empty()) {
                nameMatched = (_stricmp(name.c_str(), wantedAssetName.c_str()) == 0);
            } else {
                nameMatched = EndsWithInsensitive(name, ".dll");
            }

            if (nameMatched) {
                std::string url = JsonGetString(json, "browser_download_url", namePos);
                if (!url.empty()) return url;
            }
        }

        searchPos = namePos + nameToken.size();
    }

    return "";
}

bool ParseVersionParts(const std::string& version, std::vector<int>* outParts) {
    if (!outParts) return false;
    outParts->clear();

    size_t i = 0;
    while (i < version.size() && !std::isdigit((unsigned char)version[i])) i++;
    if (i >= version.size()) return false;

    while (i < version.size() && outParts->size() < 6) {
        if (!std::isdigit((unsigned char)version[i])) break;

        int value = 0;
        while (i < version.size() && std::isdigit((unsigned char)version[i])) {
            value = value * 10 + (version[i] - '0');
            i++;
        }
        outParts->push_back(value);

        if (i < version.size() && version[i] == '.') {
            i++;
            continue;
        }
        break;
    }

    return !outParts->empty();
}

int CompareVersionTags(const std::string& lhs, const std::string& rhs) {
    std::vector<int> lParts;
    std::vector<int> rParts;
    bool lOk = ParseVersionParts(lhs, &lParts);
    bool rOk = ParseVersionParts(rhs, &rParts);

    if (lOk && rOk) {
        size_t n = lParts.size() > rParts.size() ? lParts.size() : rParts.size();
        for (size_t i = 0; i < n; i++) {
            int lv = (i < lParts.size()) ? lParts[i] : 0;
            int rv = (i < rParts.size()) ? rParts[i] : 0;
            if (lv < rv) return -1;
            if (lv > rv) return 1;
        }
        return 0;
    }

    if (lhs == rhs) return 0;
    return (lhs < rhs) ? -1 : 1;
}

bool ReadSmallTextFile(const std::string& path, std::string* outText) {
    if (!outText) return false;
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    *outText = content;
    return true;
}

bool WriteSmallTextFile(const std::string& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;
    out.write(text.data(), (std::streamsize)text.size());
    return out.good();
}

bool HttpGetText(const std::wstring& host, const std::wstring& path, std::string* outBody) {
    if (!outBody) return false;

    HINTERNET hSession = nullptr;
    HINTERNET hConnect = nullptr;
    HINTERNET hRequest = nullptr;
    bool ok = false;
    const wchar_t* headers =
        L"User-Agent: GFRInjector\r\n"
        L"Accept: application/vnd.github+json\r\n"
        L"X-GitHub-Api-Version: 2022-11-28\r\n";
    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);

    hSession = WinHttpOpen(L"GFRInjector/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!hSession) goto cleanup;

    hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) goto cleanup;

    hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) goto cleanup;

    if (!WinHttpSendRequest(hRequest, headers, (DWORD)-1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) goto cleanup;
    if (!WinHttpReceiveResponse(hRequest, nullptr)) goto cleanup;

    if (!WinHttpQueryHeaders(hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX)) {
        goto cleanup;
    }
    if (statusCode != 200) goto cleanup;

    {
        std::string body;
        while (true) {
            DWORD available = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &available)) goto cleanup;
            if (available == 0) break;

            std::vector<char> buffer(available);
            DWORD read = 0;
            if (!WinHttpReadData(hRequest, buffer.data(), available, &read)) goto cleanup;
            if (read > 0) body.append(buffer.data(), buffer.data() + read);
        }
        *outBody = std::move(body);
    }

    ok = true;

cleanup:
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
    return ok;
}

bool DownloadFileToPath(const std::string& url, const std::string& outputPath) {
    HRESULT hr = URLDownloadToFileA(nullptr, url.c_str(), outputPath.c_str(), 0, nullptr);
    return SUCCEEDED(hr) && FileExists(outputPath);
}

std::string JoinPath(const std::string& dir, const char* fileName) {
    std::string out = dir;
    if (!out.empty() && out.back() != '\\') out.push_back('\\');
    out += fileName;
    return out;
}

bool TryAutoUpdate(const std::string& exeDir, const std::string& dllPath) {
    const std::string owner = TrimCopy(GFR_GITHUB_OWNER);
    const std::string repo = TrimCopy(GFR_GITHUB_REPO);
    std::string assetName = TrimCopy(GFR_GITHUB_ASSET);
    if (assetName.empty()) assetName = "InternalAimbot.dll";

    if (owner.empty() || repo.empty()) {
        DebugLog("[GFR Injector] Auto-update: GitHub owner/repo not configured in build.\n");
        return false;
    }

    std::string apiPath = "/repos/" + owner + "/" + repo + "/releases/latest";
    std::string latestJson;
    if (!HttpGetText(L"api.github.com", Utf8ToWide(apiPath), &latestJson)) {
        DebugLog("[GFR Injector] Auto-update: failed to fetch latest release JSON.\n");
        return false;
    }

    std::string latestTag = TrimCopy(JsonGetString(latestJson, "tag_name"));
    if (latestTag.empty()) {
        DebugLog("[GFR Injector] Auto-update: missing tag_name in release JSON.\n");
        return false;
    }

    std::string versionPath = JoinPath(exeDir, "InternalAimbot.version");
    std::string localVersion;
    if (!ReadSmallTextFile(versionPath, &localVersion)) {
        localVersion = GFR_MOD_VERSION;
    }
    localVersion = TrimCopy(localVersion);
    if (localVersion.empty()) localVersion = GFR_MOD_VERSION;

    if (CompareVersionTags(localVersion, latestTag) >= 0) {
        return false;
    }

    std::string downloadUrl = FindAssetDownloadUrl(latestJson, assetName);
    if (downloadUrl.empty()) {
        DebugLog("[GFR Injector] Auto-update: target asset not found in latest release.\n");
        return false;
    }

    std::string tempPath = dllPath + ".download";
    DeleteFileA(tempPath.c_str());

    if (!DownloadFileToPath(downloadUrl, tempPath)) {
        DebugLog("[GFR Injector] Auto-update: DLL download failed.\n");
        return false;
    }

    if (!MoveFileExA(tempPath.c_str(), dllPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
        DeleteFileA(tempPath.c_str());
        DebugLog("[GFR Injector] Auto-update: failed to replace local DLL.\n");
        return false;
    }

    WriteSmallTextFile(versionPath, latestTag + "\n");
    DebugLog("[GFR Injector] Auto-update: DLL updated from latest GitHub release.\n");
    return true;
}

DWORD GetProcessIdByName(const wchar_t* processName) {
    DWORD pid = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe = { sizeof(pe) };
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, processName) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }

    CloseHandle(snap);
    return pid;
}

bool InjectDLL(DWORD pid, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) {
        MessageBoxA(nullptr, "Failed to open process.", "GFR Injector", MB_ICONERROR);
        return false;
    }

    size_t pathLen = strlen(dllPath) + 1;
    LPVOID remotePath = VirtualAllocEx(hProcess, nullptr, pathLen, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remotePath) {
        CloseHandle(hProcess);
        MessageBoxA(nullptr, "Failed to allocate memory in target process.", "GFR Injector", MB_ICONERROR);
        return false;
    }

    if (!WriteProcessMemory(hProcess, remotePath, dllPath, pathLen, nullptr)) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        MessageBoxA(nullptr, "Failed to write DLL path into target process.", "GFR Injector", MB_ICONERROR);
        return false;
    }

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    LPVOID loadLibrary = GetProcAddress(kernel32, "LoadLibraryA");
    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
        (LPTHREAD_START_ROUTINE)loadLibrary, remotePath, 0, nullptr);

    if (!hThread) {
        VirtualFreeEx(hProcess, remotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        MessageBoxA(nullptr, "Failed to create remote thread.", "GFR Injector", MB_ICONERROR);
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

} // namespace

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    char exePath[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);

    char* lastSlash = strrchr(exePath, '\\');
    if (!lastSlash) {
        MessageBoxA(nullptr, "Failed to determine injector directory.", "GFR Injector", MB_ICONERROR);
        return 1;
    }
    *lastSlash = '\0';

    std::string exeDir = exePath;
    std::string dllPath = JoinPath(exeDir, "InternalAimbot.dll");

    // Optional auto-update from GitHub Releases.
    TryAutoUpdate(exeDir, dllPath);

    if (!FileExists(dllPath)) {
        MessageBoxA(nullptr, "InternalAimbot.dll not found!", "GFR Injector", MB_ICONERROR);
        return 1;
    }

    DWORD pid = 0;
    while (pid == 0) {
        pid = GetProcessIdByName(L"Gunfire Reborn.exe");
        if (pid == 0) Sleep(1000);
    }

    Sleep(2000);

    if (!InjectDLL(pid, dllPath.c_str())) {
        MessageBoxA(nullptr, "Injection failed!", "GFR Injector", MB_ICONERROR);
        return 1;
    }

    return 0;
}
