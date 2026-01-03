#define _USE_MATH_DEFINES
#include <cmath>
#include "aimbot.h"
#include "settings.h"

CachedTarget g_CachedTarget = {0, 0, 0, false};

EnableCtrl_t g_OriginalEnableCtrl = nullptr;
Enable_t g_OriginalEnable = nullptr;
ThrowEnableCtrl_t g_OriginalThrowEnableCtrl = nullptr;

void* g_EnableCtrlAddr = nullptr;
void* g_EnableAddr = nullptr;
void* g_ThrowEnableCtrlAddr = nullptr;

void UpdateCachedTarget(Vector3* aimStart, Vector3* aimEnd) {
    if (!g_GetMonsters || !g_GetWeakTrans || !g_GetPosition) return;
    
    __try {
        auto monsters = il2cpp_runtime_invoke(g_GetMonsters, nullptr, nullptr, nullptr);
        if (!monsters) {
            AcquireSRWLockExclusive(&g_TargetLock);
            g_CachedTarget.valid = false;
            ReleaseSRWLockExclusive(&g_TargetLock);
            return;
        }
        
        if (!g_ListGetCount) {
            auto listClass = *(Il2CppClass**)monsters;
            g_ListGetCount = il2cpp_class_get_method_from_name(listClass, "get_Count", 0);
            g_ListGetItem = il2cpp_class_get_method_from_name(listClass, "get_Item", 1);
        }
        if (!g_ListGetCount || !g_ListGetItem) return;
        
        auto countObj = il2cpp_runtime_invoke(g_ListGetCount, monsters, nullptr, nullptr);
        if (!countObj) return;
        int count = *(int*)il2cpp_object_unbox(countObj);
        
        if (count == 0) {
            AcquireSRWLockExclusive(&g_TargetLock);
            g_CachedTarget.valid = false;
            ReleaseSRWLockExclusive(&g_TargetLock);
            return;
        }
        
        if (count > 15) count = 15;
        
        Vector3 aimDir = {
            aimEnd->x - aimStart->x,
            aimEnd->y - aimStart->y,
            aimEnd->z - aimStart->z
        };
        float aimLen = sqrtf(aimDir.x*aimDir.x + aimDir.y*aimDir.y + aimDir.z*aimDir.z);
        if (aimLen > 0.001f) {
            aimDir.x /= aimLen;
            aimDir.y /= aimLen;
            aimDir.z /= aimLen;
        }
        
        Vector3 bestPos = {0, 0, 0};
        float bestAngle = 999.0f;
        bool found = false;
        
        for (int i = 0; i < count; i++) {
            void* args[] = { &i };
            auto monster = il2cpp_runtime_invoke(g_ListGetItem, monsters, args, nullptr);
            if (!monster) continue;
            
            auto bodyPartCom = *(Il2CppObject**)((char*)monster + OFFSET_BODYPARTCOM);
            if (!bodyPartCom) continue;
            
            // 우선순위: SpecialWeakness > Monster_Weakness
            auto weakTrans = GetBestWeaknessTrans(bodyPartCom);
            if (!weakTrans) continue;
            
            auto posObj = il2cpp_runtime_invoke(g_GetPosition, weakTrans, nullptr, nullptr);
            if (!posObj) continue;
            
            auto pos = (Vector3*)il2cpp_object_unbox(posObj);
            
            Vector3 toMonster = {
                pos->x - aimStart->x,
                pos->y - aimStart->y,
                pos->z - aimStart->z
            };
            float dist = sqrtf(toMonster.x*toMonster.x + toMonster.y*toMonster.y + toMonster.z*toMonster.z);
            if (dist < 0.5f) continue;
            
            toMonster.x /= dist;
            toMonster.y /= dist;
            toMonster.z /= dist;
            
            float dot = aimDir.x*toMonster.x + aimDir.y*toMonster.y + aimDir.z*toMonster.z;
            float angle = acosf(fmaxf(-1.0f, fminf(1.0f, dot))) * 57.2958f;
            
            if (angle < 60.0f && angle < bestAngle) {
                bestAngle = angle;
                bestPos = *pos;
                found = true;
            }
        }
        
        AcquireSRWLockExclusive(&g_TargetLock);
        if (found) {
            g_CachedTarget.x = bestPos.x;
            g_CachedTarget.y = bestPos.y;
            g_CachedTarget.z = bestPos.z;
            g_CachedTarget.valid = true;
        } else {
            g_CachedTarget.valid = false;
        }
        ReleaseSRWLockExclusive(&g_TargetLock);
        
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        AcquireSRWLockExclusive(&g_TargetLock);
        g_CachedTarget.valid = false;
        ReleaseSRWLockExclusive(&g_TargetLock);
    }
}

bool GetCachedTarget(Vector3* outPos) {
    if (g_ShuttingDown.load(std::memory_order_acquire)) return false;

    AcquireSRWLockShared(&g_TargetLock);
    bool valid = g_CachedTarget.valid;
    if (valid) {
        outPos->x = g_CachedTarget.x;
        outPos->y = g_CachedTarget.y;
        outPos->z = g_CachedTarget.z;
    }
    ReleaseSRWLockShared(&g_TargetLock);
    return valid;
}

void HookedEnableCtrl(void* thisPtr, void* skill, Vector3* start, Vector3* showstart, Vector3* end,
    int pierce, float distance, float speed, int targettype, void* effect, float liveTime,
    void* trailEffect, float effectLiveTime, float radius, float flyoverdis, int passid, int extCheck)
{
    if (g_ShuttingDown.load(std::memory_order_acquire) || !g_HooksInstalled.load(std::memory_order_acquire)) {
        if (g_OriginalEnableCtrl) {
            g_OriginalEnableCtrl(thisPtr, skill, start, showstart, end, pierce, distance, speed,
                targettype, effect, liveTime, trailEffect, effectLiveTime, radius, flyoverdis, passid, extCheck);
        }
        return;
    }

    if (g_SilentAimEnabled && end && targettype == 1) {
        Vector3 targetPos;
        if (GetCachedTarget(&targetPos)) {
            *end = targetPos;
        }
    }

    float finalSpeed = g_FastBullet ? speed * g_BulletSpeedMultiplier : speed;
    float finalRadius = g_BigRadius ? g_RadiusValue : radius;

    g_OriginalEnableCtrl(thisPtr, skill, start, showstart, end, pierce, distance, finalSpeed,
        targettype, effect, liveTime, trailEffect, effectLiveTime, finalRadius, flyoverdis, passid, extCheck);
}

void HookedEnable(void* thisPtr, void* skill, Vector3* start, Vector3* checkstart, Vector3* end,
    int pierce, float distance, float speed, int targettype, void* effect, float liveTime,
    void* traileffect, float effectlivetime, float radius, float flyoverdis)
{
    if (g_ShuttingDown.load(std::memory_order_acquire) || !g_HooksInstalled.load(std::memory_order_acquire)) {
        if (g_OriginalEnable) {
            g_OriginalEnable(thisPtr, skill, start, checkstart, end, pierce, distance, speed,
                targettype, effect, liveTime, traileffect, effectlivetime, radius, flyoverdis);
        }
        return;
    }

    if (g_SilentAimEnabled && end && targettype == 1) {
        Vector3 targetPos;
        if (GetCachedTarget(&targetPos)) {
            *end = targetPos;
        }
    }

    float finalSpeed = g_FastBullet ? speed * g_BulletSpeedMultiplier : speed;
    float finalRadius = g_BigRadius ? g_RadiusValue : radius;

    g_OriginalEnable(thisPtr, skill, start, checkstart, end, pierce, distance, finalSpeed,
        targettype, effect, liveTime, traileffect, effectlivetime, finalRadius, flyoverdis);
}

void HookedThrowEnableCtrl(void* thisPtr, void* skill, Vector3* start, Vector3* dir, float speed,
    float radius, Vector3* upForce, void* bounciness, int delayTrigger, bool hitOver, bool hitStaticOver,
    void* grenade, float liveTime, void* grenadeEffect, float effectLiveTime, float innerRadius,
    int pierce, bool ignoreMonster, void* hitUnitBounciness, int targettype, int hitovertype,
    float changeRadius, float maxRadius, bool ignorePetrochemical, int iTriggerBullet, bool ignoreShield,
    float checkWeaknessAngle, float checkWeaknessDis, bool canPierceWeakness, float hitFallAcc, float verticalThreshold)
{
    if (g_ShuttingDown.load(std::memory_order_acquire) || !g_HooksInstalled.load(std::memory_order_acquire)) {
        if (g_OriginalThrowEnableCtrl) {
            g_OriginalThrowEnableCtrl(thisPtr, skill, start, dir, speed, radius, upForce, bounciness,
                delayTrigger, hitOver, hitStaticOver, grenade, liveTime, grenadeEffect, effectLiveTime,
                innerRadius, pierce, ignoreMonster, hitUnitBounciness, targettype, hitovertype,
                changeRadius, maxRadius, ignorePetrochemical, iTriggerBullet, ignoreShield,
                checkWeaknessAngle, checkWeaknessDis, canPierceWeakness, hitFallAcc, verticalThreshold);
        }
        return;
    }

    float finalSpeed = speed;
    Vector3 finalDir = *dir;
    Vector3 finalUpForce = *upForce;

    if (g_SkillAimEnabled && dir && targettype == 1) {
        Vector3 targetPos;
        if (GetCachedTarget(&targetPos)) {
            Vector3 toMonster = {
                targetPos.x - start->x,
                targetPos.y - start->y,
                targetPos.z - start->z
            };
            float dist = sqrtf(toMonster.x*toMonster.x + toMonster.y*toMonster.y + toMonster.z*toMonster.z);
            if (dist > 0.5f) {
                finalDir.x = toMonster.x / dist;
                finalDir.y = toMonster.y / dist;
                finalDir.z = toMonster.z / dist;
                finalSpeed = speed * g_SkillSpeedMultiplier;
                finalUpForce.x = 0;
                finalUpForce.y = 0;
                finalUpForce.z = 0;
            }
        }
    }

    g_OriginalThrowEnableCtrl(thisPtr, skill, start, &finalDir, finalSpeed, radius, &finalUpForce, bounciness,
        delayTrigger, hitOver, hitStaticOver, grenade, liveTime, grenadeEffect, effectLiveTime,
        innerRadius, pierce, ignoreMonster, hitUnitBounciness, targettype, hitovertype,
        changeRadius, maxRadius, ignorePetrochemical, iTriggerBullet, ignoreShield,
        checkWeaknessAngle, checkWeaknessDis, canPierceWeakness, hitFallAcc, verticalThreshold);
}
