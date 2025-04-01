#pragma once
#include "cbase.h"
#include "cs_player.h"
#include "weapon_csbase.h"

#define MAX_ANGLE_DELTA 25.0f    // Maksymalna akceptowalna zmiana kątu w 1 klatce
#define TRIGGERBOT_THRESHOLD 0.1f // 100 ms
#define GROUND_LOOK_THRESHOLD 45.0f // Kąt patrzenia w ziemię

class AntiCheatSystem {
private:
    struct PlayerData {
        QAngle prevAngles;
        float lastTargetTime = 0.0f;
        int suspiciousActions = 0;
    };

    CUtlMap<C_CSPlayer*, PlayerData> playerData;
    float GetCurrentTime() const;

public:
    AntiCheatSystem();
    void OnPlayerFire(C_CSPlayer* player);
    void OnGameFrame();
    void CheckForAimbot(C_CSPlayer* player);
    void CheckForTriggerbot(C_CSPlayer* player);
    void CheckGroundAim(C_CSPlayer* player);
    void FlagPlayer(C_CSPlayer* player, const char* reason);
};

extern AntiCheatSystem g_AntiCheat;
