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
// SkillBolt.DelegateThrowCartoon.EnableCtrl (16 params)
typedef void (*ThrowEnableCtrl_t)(void*, void*, Vector3*, Vector3*, float, float, Vector3*, void*, int, bool, void*, float, void*, float, float, int, float);
// SkillBolt.EntityParabolaCartoon.EnableCtrl (22 params)
typedef void (*ParabolaEnableCtrl_t)(void*, void*, Vector3*, Vector3*, float, float, Vector3*, void*, void*, int, bool, bool, int, bool, int, int, int, float, bool, bool, float, float, bool);

// Original function pointers
extern EnableCtrl_t g_OriginalEnableCtrl;
extern Enable_t g_OriginalEnable;
extern ThrowEnableCtrl_t g_OriginalThrowEnableCtrl;
extern ParabolaEnableCtrl_t g_OriginalParabolaEnableCtrl;

// Hook addresses
extern void* g_EnableCtrlAddr;
extern void* g_EnableAddr;
extern void* g_ThrowEnableCtrlAddr;
extern void* g_ParabolaEnableCtrlAddr;

// Hooked functions
void HookedEnableCtrl(void* thisPtr, void* skill, Vector3* start, Vector3* showstart, Vector3* end,
    int pierce, float distance, float speed, int targettype, void* effect, float liveTime,
    void* trailEffect, float effectLiveTime, float radius, float flyoverdis, int passid, int extCheck);

void HookedEnable(void* thisPtr, void* skill, Vector3* start, Vector3* checkstart, Vector3* end,
    int pierce, float distance, float speed, int targettype, void* effect, float liveTime,
    void* traileffect, float effectlivetime, float radius, float flyoverdis);

void HookedThrowEnableCtrl(void* thisPtr, void* skill, Vector3* start, Vector3* dir, float speed,
    float radius, Vector3* upForce, void* bounciness, int delayTrigger, bool hitStaticOver,
    void* grenade, float liveTime, void* trailEffect, float effectLiveTime, float innerRadius,
    int pierce, float effectScale);

void HookedParabolaEnableCtrl(void* thisPtr, void* skill, Vector3* start, Vector3* dir, float speed,
    float radius, Vector3* accelerate, void* bounciness, void* hitUnitBounciness, int delayTrigger,
    bool hitOver, bool hitStaticOver, int pierce, bool ignoreMonster, int summonID, int summonSID,
    int targetType, float maxDistance, bool hitHeroOver, bool forceSpeed, float innerRadius,
    float scale, bool hitMonsterOver);
