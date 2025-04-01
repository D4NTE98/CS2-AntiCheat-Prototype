DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING reg) {
    UNICODE_STRING dev_name = RTL_CONSTANT_STRING(L"\\Device\\VACDriver");
    PDEVICE_OBJECT dev_obj;
    NTSTATUS status = IoCreateDevice(driver, 0, &dev_name, FILE_DEVICE_UNKNOWN, 0, FALSE, &dev_obj);
    
    driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = VacDeviceControl;
    driver->DriverUnload = DriverUnload;
    
    InitSignatureDatabase();
    return status;
}

NTSTATUS VacDeviceControl(PDEVICE_OBJECT dev, PIRP irp) {
    auto stack = IoGetCurrentIrpStackLocation(irp);
    auto buffer = irp->AssociatedIrp.SystemBuffer;
    
    switch (stack->Parameters.DeviceIoControl.IoControlCode) {
        case VAC_IOCTL_SCAN: {
            VacIpcData* data = (VacIpcData*)buffer;
            if (DetectCheatSignature(data->base_address, data->region_size)) {
                ReportViolation(data->cheat_signature);
            }
            break;
        }
    }
    
    irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

bool DetectCheatSignature(void* address, size_t size) {
    const uint32_t known_signatures[] = {0xDEADBEEF, 0xCAFEBABE};
    uint32_t region_hash = CalculateMemoryHash(address, size);
    
    for (auto sig : known_signatures) {
        if (region_hash == sig) return true;
    }
    return false;
}

void ReportViolation(uint32_t signature) {
    ULONG_PTR args[1] = {signature};
    KeInsertQueueApc(GetUserApc(), args, NULL, 0);
}
