#include "DLLIntegrityChecker.h"
#include <iostream>
#include <fstream>

DLLIntegrityChecker::DLLIntegrityChecker(AntiCheatSystem& antiCheat)
    : m_antiCheat(antiCheat) {
    InitializeCriticalSection(&m_cs);
    InitializeKnownDLLs();
}

DLLIntegrityChecker::~DLLIntegrityChecker() {
    StopMonitoring();
    DeleteCriticalSection(&m_cs);
}

void DLLIntegrityChecker::InitializeKnownDLLs() {
    const wchar_t* modules[] = {
        L"client.dll",
        L"engine.dll",
        L"shaderapidx9.dll"
    };

    for (const auto& module : modules) {
        HMODULE hMod = NULL;
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            module, &hMod)) {
            
            DLLInfo info;
            info.path = GetModulePath(hMod);
            
            // Get initial size and hash
            WIN32_FILE_ATTRIBUTE_DATA fad;
            if (GetFileAttributesExW(info.path.c_str(), GetFileExInfoStandard, &fad)) {
                info.originalSize = fad.nFileSizeLow;
                ComputeFileHash(info.path, info.originalHash);
                m_knownDLLs.push_back(info);
            }
        }
    }
}

void DLLIntegrityChecker::StartMonitoring() {
    m_running = true;
    m_monitorThread = std::thread(&DLLIntegrityChecker::MonitorLoop, this);
}

void DLLIntegrityChecker::StopMonitoring() {
    m_running = false;
    if (m_monitorThread.joinable()) {
        m_monitorThread.join();
    }
}

void DLLIntegrityChecker::MonitorLoop() {
    while (m_running) {
        EnterCriticalSection(&m_cs);
        
        for (const auto& dll : m_knownDLLs) {
            if (!VerifyDLLIntegrity(dll)) {
                m_antiCheat.FlagPlayer(nullptr, 
                    "DLL Integrity violation detected in " + std::string(dll.path.begin(), dll.path.end()));
            }
        }
        
        LeaveCriticalSection(&m_cs);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

bool DLLIntegrityChecker::ComputeFileHash(const std::wstring& path, std::vector<BYTE>& hash) {
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    constexpr DWORD bufferSize = 4096;
    BYTE buffer[bufferSize];
    
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        return false;
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return false;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }

    while (file) {
        file.read(reinterpret_cast<char*>(buffer), bufferSize);
        DWORD read = static_cast<DWORD>(file.gcount());
        if (!CryptHashData(hHash, buffer, read, 0)) {
            break;
        }
    }

    DWORD hashSize = 0;
    DWORD dwSize = sizeof(DWORD);
    CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&hashSize, &dwSize, 0);

    hash.resize(hashSize);
    CryptGetHashParam(hHash, HP_HASHVAL, hash.data(), &hashSize, 0);

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return true;
}

bool DLLIntegrityChecker::VerifyDLLIntegrity(const DLLInfo& dll) {
    // Check file size
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(dll.path.c_str(), GetFileExInfoStandard, &fad)) {
        return false;
    }

    if (fad.nFileSizeLow != dll.originalSize) {
        return false;
    }

    // Check hash
    std::vector<BYTE> currentHash;
    if (!ComputeFileHash(dll.path, currentHash)) {
        return false;
    }

    return currentHash == dll.originalHash;
}

std::wstring DLLIntegrityChecker::GetModulePath(HMODULE module) {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(module, path, MAX_PATH);
    return path;
}
