#pragma once
#include <ntifs.h>

#define VAC_DEVICE 0x8000
#define VAC_IOCTL_SCAN CTL_CODE(VAC_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

struct VacIpcData {
    void* base_address;
    size_t region_size;
    uint32_t cheat_signature;
};
