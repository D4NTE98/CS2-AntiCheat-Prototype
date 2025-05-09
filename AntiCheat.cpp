#include "AntiCheat.h"

AntiCheatSystem g_AntiCheat;

AntiCheatSystem::AntiCheatSystem() : playerData(DefLessFunc(C_CSPlayer*)) {}

float AntiCheatSystem::GetCurrentTime() const {
    return gpGlobals->curtime;
}

void AntiCheatSystem::OnGameFrame() {
    FOR_EACH_VALID_SPLITSCREEN_PLAYER(hSlot) {
        C_CSPlayer* pPlayer = C_CSPlayer::GetLocalCSPlayer(hSlot);
        if (!pPlayer) continue;
        
        trace_t tr;
        Vector vecStart, vecEnd;
        pPlayer->EyePositionAndVectors(&vecStart, nullptr, nullptr, nullptr);
        vecEnd = vecStart + pPlayer->GetLookVectors() * 8192.0f;
        
        UTIL_TraceLine(vecStart, vecEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);
        
        if (tr.DidHit() && tr.m_pEnt && tr.m_pEnt->IsPlayer()) {
            C_CSPlayer* target = ToCSPlayer(tr.m_pEnt);
            if (target && target->GetTeamNumber() != pPlayer->GetTeamNumber()) {
                PlayerData* data = &playerData[pPlayer];
                data->lastTargetTime = GetCurrentTime();
            }
        }
    }
}

void AntiCheatSystem::OnPlayerFire(C_CSPlayer* player) {
    memoryGuard.AntiDebuggingCheck();
    memoryGuard.VerifyMemoryIntegrity();
    
    CheckForAimbot(player);
    CheckForTriggerbot(player);
    CheckGroundAim(player);
    CollectMLData(player);
    CheckForMLAnomalies(player);
}

void AntiCheatSystem::CheckForAimbot(C_CSPlayer* player) {
    PlayerData* data = &playerData[player];
    QAngle currentAngles = player->GetEyeAngles();
    
    float deltaYaw = fabs(currentAngles.yaw - data->prevAngles.yaw);
    float deltaPitch = fabs(currentAngles.pitch - data->prevAngles.pitch);

    if (deltaYaw > 180.0f) deltaYaw = 360.0f - deltaYaw;
    if (deltaPitch > 180.0f) deltaPitch = 360.0f - deltaPitch;

    if (deltaYaw > MAX_ANGLE_DELTA || deltaPitch > MAX_ANGLE_DELTA) {
        if (++data->suspiciousActions > 3) {
            FlagPlayer(player, "Suspected Aimbot: Sudden view angle change");
        }
    }
    data->prevAngles = currentAngles;
}

void AntiCheatSystem::CheckForTriggerbot(C_CSPlayer* player) {
    PlayerData* data = &playerData[player];
    float reactionTime = GetCurrentTime() - data->lastTargetTime;
    
    if (reactionTime < TRIGGERBOT_THRESHOLD) {
        if (++data->suspiciousActions > 2) {
            FlagPlayer(player, "Suspected Triggerbot: Instant reaction time");
        }
    }
}

void AntiCheatSystem::CheckGroundAim(C_CSPlayer* player) {
    QAngle angles = player->GetEyeAngles();
    if (angles.pitch > GROUND_LOOK_THRESHOLD) {
        FlagPlayer(player, "Suspected Ground Aim: Shooting while looking at ground");
    }
}

void AntiCheatSystem::FlagPlayer(C_CSPlayer* player, const char* reason) {
    Warning("Anti-Cheat flag: Player %s - %s\n", player->GetPlayerName(), reason);
    engine->ServerCmd("kickid %d %s;", player->GetUserID(), reason);
}

// Hooking do istniejącego kodu strzelania
class AntiCheatHook_FireBullets {
public:
    void FireBulletsHook(const FireBulletsInfo_t& info) {
        C_CSPlayer* player = GetPlayer();
        g_AntiCheat.OnPlayerFire(player);
        OriginalFunction(info);
    }

    static void __fastcall Hooked_FireBullets(void* thisptr, void* edx, const FireBulletsInfo_t& info) {
        ((AntiCheatHook_FireBullets*)thisptr)->FireBulletsHook(info);
    }
    
    ORIGINAL_FUNCTION_PTR(FireBullets, void*, (const FireBulletsInfo_t&));
};

void InstallAntiCheatHooks() {
    AntiCheatHook_FireBullets::Hook();
    g_AntiCheat.InitializeMemoryProtection();
    g_AntiCheat.mlModel.LoadModel("models/anticheat_model.tflite");
    
    EncryptMemoryRegion(g_AntiCheat.memoryGuard);
}

void AntiCheatSystem::CollectMLData(C_CSPlayer* player) {
    BehavioralFeatures features;
    
    // Zbieranie danych o ruchu myszy
    features.aimSpeedVariance = CalculateAimVariance(player);
    
    // Analiza czasu między strzałami
    GetShotTimingStatistics(player, features.shotTimeDiffs);
    
    // Obliczanie entropii ruchów myszy
    features.mouseMovementEntropy = CalculateEntropy(player->GetMouseSamples());
    
    mlModel.AddToInferenceQueue(features);
}

template<typename T>
class EncryptedValue {
private:
    uint64_t xorKey;
    T encryptedData;

public:
    EncryptedValue(T value) : xorKey(GenerateRandomKey()) {
        encryptedData = Encrypt(value, xorKey);
    }

    T Get() const {
        return Decrypt(encryptedData, xorKey);
    }

    void Set(T value) {
        encryptedData = Encrypt(value, xorKey);
    }

    static T Encrypt(T data, uint64_t key) {
        // ... implementacja szyfrowania XOR z rotacją
    }
};

EncryptedValue<float> detectionThreshold{0.85f};

// ml
HOOK(void*, __fastcall, Hooked_memcpy, (void* dest, const void* src, size_t size)) {
    if (IsProtectedRegion(dest)) {
        ReportMemoryViolation();
        return dest;
    }
    return original_Hooked_memcpy(dest, src, size);
}

void InstallSystemHooks() {
    HOOK_FUNCTION(kernel32.dll, memcpy, Hooked_memcpy);
}

void ScanForForeignDLLs() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    MODULEENTRY32 moduleEntry;
    
    while (Module32Next(snapshot, &moduleEntry)) {
        if (!IsValidModule(moduleEntry.szModule)) {
            ReportUnauthorizedModule(moduleEntry.szModule);
        }
    }
}
