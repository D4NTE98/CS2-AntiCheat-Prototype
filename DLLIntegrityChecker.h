#pragma once
#include <Windows.h>
#include <wincrypt.h>
#include <vector>
#include <thread>
#include <atomic>
#include "AntiCheatSystem.h"

class DLLIntegrityChecker {
public:
    struct DLLInfo {
        std::wstring path;
        DWORD originalSize;
        std::vector<BYTE> originalHash;
    };

    DLLIntegrityChecker(AntiCheatSystem& antiCheat);
    ~DLLIntegrityChecker();

    void StartMonitoring();
    void StopMonitoring();

private:
    AntiCheatSystem& m_antiCheat;
    std::atomic<bool> m_running{false};
    std::thread m_monitorThread;
    std::vector<DLLInfo> m_knownDLLs;
    CRITICAL_SECTION m_cs;

    void InitializeKnownDLLs();
    void MonitorLoop();
    bool ComputeFileHash(const std::wstring& path, std::vector<BYTE>& hash);
    bool VerifyDLLIntegrity(const DLLInfo& dll);
    static std::wstring GetModulePath(HMODULE module);
};
