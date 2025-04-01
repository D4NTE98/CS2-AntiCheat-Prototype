void PerformAntiDebugChecks() {
    CheckKernelDebugger();
    CheckHardwareBreakpoints();
    VerifyDebugRegisters();
    CheckProcessDebugFlags();
    
    auto start = __rdtsc();
    ExecuteDecoyCode();
    auto duration = __rdtsc() - start;
    if (duration > THRESHOLD) ReportDebugging();
}

__declspec(noinline) void ExecuteDecoyCode() {
    volatile int junk = 0;
    for (int i = 0; i < 1000; ++i) {
        junk ^= i;
    }
}
