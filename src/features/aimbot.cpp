#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <cstdint>
#include "aimbot.h"
#include "settings.h"

CachedTarget g_CachedTarget = {0, 0, 0, false};

EnableCtrl_t g_OriginalEnableCtrl = nullptr;
Enable_t g_OriginalEnable = nullptr;
ThrowEnableCtrl_t g_OriginalThrowEnableCtrl = nullptr;
ParabolaEnableCtrl_t g_OriginalParabolaEnableCtrl = nullptr;

void* g_EnableCtrlAddr = nullptr;
void* g_EnableAddr = nullptr;
void* g_ThrowEnableCtrlAddr = nullptr;
void* g_ParabolaEnableCtrlAddr = nullptr;

namespace {
Il2CppMethod* g_PhysicsLinecast2 = nullptr;
bool g_PhysicsLinecastResolved = false;

void ResolvePhysicsLinecast() {
    if (g_PhysicsLinecastResolved) return;
    g_PhysicsLinecastResolved = true;

    if (!il2cpp_class_from_name || !il2cpp_class_get_method_from_name) return;

    Il2CppImage* imgPhysics = FindImage("UnityEngine.PhysicsModule");
    if (!imgPhysics) return;

    Il2CppClass* physicsClass = il2cpp_class_from_name(imgPhysics, "UnityEngine", "Physics");
    if (!physicsClass) return;

    // Use the unambiguous overload: Physics.Linecast(Vector3 start, Vector3 end)
    g_PhysicsLinecast2 = il2cpp_class_get_method_from_name(physicsClass, "Linecast", 2);
    if (!g_PhysicsLinecast2) {
        printf("[GFR Mod] Aimbot LOS: Physics.Linecast(start,end) not found\n");
    }
}

bool IsTargetOccluded(const Vector3& from, const Vector3& target) {
    ResolvePhysicsLinecast();
    if (!g_PhysicsLinecast2 || !il2cpp_runtime_invoke || !il2cpp_object_unbox) {
        // Fail-open: if LOS API is unavailable, keep previous behavior.
        return false;
    }

    Vector3 delta = {
        target.x - from.x,
        target.y - from.y,
        target.z - from.z
    };
    float dist = sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if (dist < 1.0f) return false;

    // Pull endpoint slightly toward camera to avoid hitting target collider itself.
    float invDist = 1.0f / dist;
    Vector3 end = {
        target.x - delta.x * invDist * 0.35f,
        target.y - delta.y * invDist * 0.35f,
        target.z - delta.z * invDist * 0.35f
    };

    void* args[] = { (void*)&from, (void*)&end };
    void* exception = nullptr;
    Il2CppObject* hitObj = il2cpp_runtime_invoke(g_PhysicsLinecast2, nullptr, args, &exception);
    if (exception || !hitObj) {
        return false;
    }

    __try {
        uint8_t hit = *(uint8_t*)il2cpp_object_unbox(hitObj);
        return hit != 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
} // namespace

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
                if (IsTargetOccluded(*aimStart, *pos)) {
                    continue;
                }
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

    g_OriginalEnableCtrl(thisPtr, skill, start, showstart, end, pierce, distance, finalSpeed,
        targettype, effect, liveTime, trailEffect, effectLiveTime, radius, flyoverdis, passid, extCheck);
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

    g_OriginalEnable(thisPtr, skill, start, checkstart, end, pierce, distance, finalSpeed,
        targettype, effect, liveTime, traileffect, effectlivetime, radius, flyoverdis);
}

void HookedThrowEnableCtrl(void* thisPtr, void* skill, Vector3* start, Vector3* dir, float speed,
    float radius, Vector3* upForce, void* bounciness, int delayTrigger, bool hitStaticOver,
    void* grenade, float liveTime, void* trailEffect, float effectLiveTime, float innerRadius,
    int pierce, float effectScale)
{
    if (g_ShuttingDown.load(std::memory_order_acquire) || !g_HooksInstalled.load(std::memory_order_acquire)) {
        if (g_OriginalThrowEnableCtrl) {
            g_OriginalThrowEnableCtrl(thisPtr, skill, start, dir, speed, radius, upForce, bounciness,
                delayTrigger, hitStaticOver, grenade, liveTime, trailEffect, effectLiveTime,
                innerRadius, pierce, effectScale);
        }
        return;
    }

    float finalSpeed = speed;
    Vector3 finalDir = dir ? *dir : Vector3{0, 0, 0};
    Vector3 finalUpForce = upForce ? *upForce : Vector3{0, 0, 0};

    g_OriginalThrowEnableCtrl(thisPtr, skill, start, &finalDir, finalSpeed, radius, &finalUpForce, bounciness,
        delayTrigger, hitStaticOver, grenade, liveTime, trailEffect, effectLiveTime,
        innerRadius, pierce, effectScale);
}

void HookedParabolaEnableCtrl(void* thisPtr, void* skill, Vector3* start, Vector3* dir, float speed,
    float radius, Vector3* accelerate, void* bounciness, void* hitUnitBounciness, int delayTrigger,
    bool hitOver, bool hitStaticOver, int pierce, bool ignoreMonster, int summonID, int summonSID,
    int targetType, float maxDistance, bool hitHeroOver, bool forceSpeed, float innerRadius,
    float scale, bool hitMonsterOver)
{
    if (g_ShuttingDown.load(std::memory_order_acquire) || !g_HooksInstalled.load(std::memory_order_acquire)) {
        if (g_OriginalParabolaEnableCtrl) {
            g_OriginalParabolaEnableCtrl(thisPtr, skill, start, dir, speed, radius, accelerate, bounciness,
                hitUnitBounciness, delayTrigger, hitOver, hitStaticOver, pierce, ignoreMonster,
                summonID, summonSID, targetType, maxDistance, hitHeroOver, forceSpeed, innerRadius,
                scale, hitMonsterOver);
        }
        return;
    }

    float finalSpeed = speed;
    Vector3 finalDir = dir ? *dir : Vector3{0, 0, 0};
    Vector3 finalAccelerate = accelerate ? *accelerate : Vector3{0, 0, 0};

    g_OriginalParabolaEnableCtrl(thisPtr, skill, start, &finalDir, finalSpeed, radius, &finalAccelerate, bounciness,
        hitUnitBounciness, delayTrigger, hitOver, hitStaticOver, pierce, ignoreMonster,
        summonID, summonSID, targetType, maxDistance, hitHeroOver, forceSpeed, innerRadius,
        scale, hitMonsterOver);
}
