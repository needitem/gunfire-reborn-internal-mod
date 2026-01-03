#pragma once
#include "../game/game.h"

// Cached target structure
struct CachedTarget {
    float x;
    float y;
    float z;
    bool valid;
};

extern CachedTarget g_CachedTarget;

// Aimbot functions
void UpdateCachedTarget(Vector3* aimStart, Vector3* aimEnd);
bool GetCachedTarget(Vector3* outPos);

// Hook function types
typedef void (*EnableCtrl_t)(void*, void*, Vector3*, Vector3*, Vector3*, int, float, float, int, void*, float, void*, float, float, float, int, int);
typedef void (*Enable_t)(void*, void*, Vector3*, Vector3*, Vector3*, int, float, float, int, void*, float, void*, float, float, float);
typedef void (*ThrowEnableCtrl_t)(void*, void*, Vector3*, Vector3*, float, float, Vector3*, void*, int, bool, bool, void*, float, void*, float, float, int, bool, void*, int, int, float, float, bool, int, bool, float, float, bool, float, float);

// Original function pointers
extern EnableCtrl_t g_OriginalEnableCtrl;
extern Enable_t g_OriginalEnable;
extern ThrowEnableCtrl_t g_OriginalThrowEnableCtrl;

// Hook addresses
extern void* g_EnableCtrlAddr;
extern void* g_EnableAddr;
extern void* g_ThrowEnableCtrlAddr;

// Hooked functions
void HookedEnableCtrl(void* thisPtr, void* skill, Vector3* start, Vector3* showstart, Vector3* end,
    int pierce, float distance, float speed, int targettype, void* effect, float liveTime,
    void* trailEffect, float effectLiveTime, float radius, float flyoverdis, int passid, int extCheck);

void HookedEnable(void* thisPtr, void* skill, Vector3* start, Vector3* checkstart, Vector3* end,
    int pierce, float distance, float speed, int targettype, void* effect, float liveTime,
    void* traileffect, float effectlivetime, float radius, float flyoverdis);

void HookedThrowEnableCtrl(void* thisPtr, void* skill, Vector3* start, Vector3* dir, float speed,
    float radius, Vector3* upForce, void* bounciness, int delayTrigger, bool hitOver, bool hitStaticOver,
    void* grenade, float liveTime, void* grenadeEffect, float effectLiveTime, float innerRadius,
    int pierce, bool ignoreMonster, void* hitUnitBounciness, int targettype, int hitovertype,
    float changeRadius, float maxRadius, bool ignorePetrochemical, int iTriggerBullet, bool ignoreShield,
    float checkWeaknessAngle, float checkWeaknessDis, bool canPierceWeakness, float hitFallAcc, float verticalThreshold);
