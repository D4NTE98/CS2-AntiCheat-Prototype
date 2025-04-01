#pragma once
#include <vector>
#include <cstdint>
#include <Windows.h>

class MemoryGuardian {
private:
    struct ProtectedRegion {
        void* baseAddress;
        size_t size;
        uint32_t originalChecksum;
    };

    std::vector<ProtectedRegion> protectedRegions;
    
    uint32_t CalculateCRC32(void* data, size_t size) {
        //todo crc32
    }

public:
    void AddProtectedRegion(void* addr, size_t size) {
        ProtectedRegion region;
        region.baseAddress = addr;
        region.size = size;
        region.originalChecksum = CalculateCRC32(addr, size);
        protectedRegions.push_back(region);
    }

    void VerifyMemoryIntegrity() {
        for (auto& region : protectedRegions) {
            uint32_t currentChecksum = CalculateCRC32(region.baseAddress, region.size);
            if (currentChecksum != region.originalChecksum) {
                ReportMemoryTampering();
            }
        }
    }

    void AntiDebuggingCheck() {
        if (IsDebuggerPresent()) {
            TerminateProcess(GetCurrentProcess(), 0);
        }
    }
};
