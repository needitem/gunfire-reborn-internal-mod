#include "game.h"
#include "../features/norecoil.h"
#include "../features/aimbot.h"
#include "../features/fov.h"
#include "../features/settings.h"
#include <cstdio>

// Helper: resolve a method's native pointer via IL2CPP API
// MethodInfo struct has methodPointer as its first field (offset 0)
static void* ResolveMethodPtr(Il2CppImage* img, const char* ns, const char* className, const char* methodName, int paramCount) {
    if (!img) {
        printf("[GFR Mod] Image null for %s.%s\n", ns, className);
        return nullptr;
    }
    auto klass = il2cpp_class_from_name(img, ns, className);
    if (!klass) {
        printf("[GFR Mod] Class not found: %s.%s\n", ns, className);
        return nullptr;
    }
    auto method = il2cpp_class_get_method_from_name(klass, methodName, paramCount);
    if (!method) {
        printf("[GFR Mod] Method not found: %s.%s.%s(%d)\n", ns, className, methodName, paramCount);
        return nullptr;
    }
    // Try il2cpp_method_get_pointer first, fallback to reading MethodInfo.methodPointer directly
    void* ptr = nullptr;
    if (il2cpp_method_get_pointer) {
        ptr = il2cpp_method_get_pointer(method);
    }
    if (!ptr) {
        // MethodInfo.methodPointer is at offset 0
        ptr = *(void**)method;
    }
    if (!ptr) {
        printf("[GFR Mod] Failed to get pointer: %s.%s.%s\n", ns, className, methodName);
    }
    return ptr;
}

// Cached methods
Il2CppMethod* g_GetMonsters = nullptr;
Il2CppMethod* g_GetPlayer = nullptr;
Il2CppMethod* g_GetMaxBullet = nullptr;
Il2CppMethod* g_GetCurBullet = nullptr;
Il2CppMethod* g_SetCurBullet = nullptr;
Il2CppMethod* g_SetClientCurBullet = nullptr;
Il2CppMethod* g_GetSpeed = nullptr;
Il2CppMethod* g_SetSpeed = nullptr;
Il2CppMethod* g_GetJumpHeight = nullptr;
Il2CppMethod* g_SetJumpHeight = nullptr;
Il2CppMethod* g_OneGPU = nullptr;
Il2CppMethod* g_StartGPU = nullptr;
Il2CppMethod* g_GetWeakTrans = nullptr;
Il2CppMethod* g_GetPosition = nullptr;
Il2CppMethod* g_GetTransform = nullptr;
Il2CppMethod* g_GetForward = nullptr;
Il2CppMethod* g_GetMainCameraCom = nullptr;
Il2CppMethod* g_ListGetCount = nullptr;
Il2CppMethod* g_ListGetItem = nullptr;
Il2CppMethod* g_SetNoCostBullet = nullptr;
Il2CppMethod* g_GetSpecialWeakTrans = nullptr;
Il2CppMethod* g_GetWeakTransByTag = nullptr;

// Cached classes/fields
Il2CppClass* g_NewPlayerManager = nullptr;
Il2CppClass* g_NewItemProp = nullptr;
Il2CppClass* g_PlayerProp = nullptr;
Il2CppClass* g_DropManager = nullptr;
Il2CppClass* g_OCBodyPart = nullptr;
Il2CppClass* g_GMStateManager = nullptr;
void* g_MainCtrlField = nullptr;
void* g_PlayerDictField = nullptr;

// Direct function pointers
GetPositionInjected_t g_GetPositionInjected = nullptr;
SetPositionInjected_t g_SetPositionInjected = nullptr;
GetMatrix_t g_GetWorldToCameraMatrix = nullptr;
GetMatrix_t g_GetProjectionMatrix = nullptr;
GetMainCameraComDirect_t g_GetMainCameraComDirect = nullptr;
GetWarCash_t g_GetWarCash = nullptr;
SetWarCash_t g_SetWarCash = nullptr;
GetPlayerProp_t g_GetPlayerPropFunc = nullptr;
GetFOV_t g_GetFOV = nullptr;
SetFOV_t g_SetFOV = nullptr;

// Infinite ammo hook
void* g_GetNoCostBulletAddr = nullptr;
GetNoCostBullet_t g_OriginalGetNoCostBullet = nullptr;

// Weakness hit hack
CartoonDataSetSkilllRay_t g_OriginalCartoonDataSetSkilllRay = nullptr;
CartoonDataPacketSkillRay_t g_OriginalCartoonDataPacketSkillRay = nullptr;
SClientHitInfoCtor_t g_OriginalSClientHitInfoCtor = nullptr;

void* g_CartoonDataSetSkilllRayAddr = nullptr;
void* g_CartoonDataPacketSkillRayAddr = nullptr;
void* g_SClientHitInfoCtorAddr = nullptr;

void* g_WeaknessString = nullptr;
void* g_SpecialWeaknessString = nullptr;

bool InitGame() {
    auto imgCSharp = FindImage("Assembly-CSharp");
    auto imgCore = FindImage("UnityEngine.CoreModule");
    if (!imgCSharp || !imgCore) return false;
    
    g_NewPlayerManager = il2cpp_class_from_name(imgCSharp, "", "NewPlayerManager");
    g_NewItemProp = il2cpp_class_from_name(imgCSharp, "", "NewItemProp");
    g_PlayerProp = il2cpp_class_from_name(imgCSharp, "", "PlayerProp");
    g_DropManager = il2cpp_class_from_name(imgCSharp, "", "DropManager");
    g_OCBodyPart = il2cpp_class_from_name(imgCSharp, "", "OCBodyPart");
    auto Transform = il2cpp_class_from_name(imgCore, "UnityEngine", "Transform");
    
    if (!g_NewPlayerManager) return false;
    
    g_GetMonsters = il2cpp_class_get_method_from_name(g_NewPlayerManager, "GetMonsters", 0);
    g_GetPlayer = il2cpp_class_get_method_from_name(g_NewPlayerManager, "GetPlayer", 1);
    
    if (g_NewItemProp) {
        g_GetMaxBullet = il2cpp_class_get_method_from_name(g_NewItemProp, "get_MaxBullet", 0);
        g_GetCurBullet = il2cpp_class_get_method_from_name(g_NewItemProp, "get_CurBullet", 0);
        g_SetCurBullet = il2cpp_class_get_method_from_name(g_NewItemProp, "set_CurBullet", 1);
        g_SetClientCurBullet = il2cpp_class_get_method_from_name(g_NewItemProp, "set_ClientCurBullet", 1);
    }
    
    if (g_PlayerProp) {
        g_GetSpeed = il2cpp_class_get_method_from_name(g_PlayerProp, "get_Speed", 0);
        g_SetSpeed = il2cpp_class_get_method_from_name(g_PlayerProp, "set_Speed", 1);
        g_GetJumpHeight = il2cpp_class_get_method_from_name(g_PlayerProp, "get_JumpHeight", 0);
        g_SetJumpHeight = il2cpp_class_get_method_from_name(g_PlayerProp, "set_JumpHeight", 1);
    }
    
    if (g_DropManager) {
        g_OneGPU = il2cpp_class_get_method_from_name(g_DropManager, "OneGPU", 0);
        g_StartGPU = il2cpp_class_get_method_from_name(g_DropManager, "StartGPU", 1);
    }
    
    if (g_OCBodyPart) {
        g_GetWeakTrans = il2cpp_class_get_method_from_name(g_OCBodyPart, "GetWeakTrans", 1);
        g_GetSpecialWeakTrans = il2cpp_class_get_method_from_name(g_OCBodyPart, "GetSpecialWeakTrans", 0);
        g_GetWeakTransByTag = il2cpp_class_get_method_from_name(g_OCBodyPart, "GetWeakTransByTag", 1);
    }
    
    if (Transform) {
        g_GetPosition = il2cpp_class_get_method_from_name(Transform, "get_position", 0);
        g_GetForward = il2cpp_class_get_method_from_name(Transform, "get_forward", 0);
    }
    
    auto Component = il2cpp_class_from_name(imgCore, "UnityEngine", "Component");
    if (Component) {
        g_GetTransform = il2cpp_class_get_method_from_name(Component, "get_transform", 0);
    }
    
    auto CameraManager = il2cpp_class_from_name(imgCSharp, "", "CameraManager");
    if (CameraManager) {
        g_GetMainCameraCom = il2cpp_class_get_method_from_name(CameraManager, "get_MainCameraCom", 0);
    }

    if (il2cpp_class_get_field_from_name && g_NewPlayerManager) {
        g_MainCtrlField = il2cpp_class_get_field_from_name(g_NewPlayerManager, "MainCtrl");
        g_PlayerDictField = il2cpp_class_get_field_from_name(g_NewPlayerManager, "PlayerDict");
    }

    // GMStateManager - hook GetNoCostBullet for infinite ammo
    g_GMStateManager = il2cpp_class_from_name(imgCSharp, "", "GMStateManager");
    if (g_GMStateManager) {
        g_GetNoCostBulletAddr = ResolveMethodPtr(imgCSharp, "", "GMStateManager", "GetNoCostBullet", 0);
        printf("[GFR Mod] GetNoCostBullet resolved: %p\n", g_GetNoCostBulletAddr);
    } else {
        printf("[GFR Mod] GMStateManager class NOT found\n");
    }

    // --- Resolve all method pointers at runtime via IL2CPP API (no hardcoded RVAs) ---
    auto imgPhysics = FindImage("UnityEngine.PhysicsModule");

    // Unity engine function pointers
    g_GetPositionInjected = (GetPositionInjected_t)ResolveMethodPtr(imgCore, "UnityEngine", "Transform", "get_position_Injected", 1);
    g_SetPositionInjected = (SetPositionInjected_t)ResolveMethodPtr(imgCore, "UnityEngine", "Transform", "set_position_Injected", 1);
    g_GetWorldToCameraMatrix = (GetMatrix_t)ResolveMethodPtr(imgCore, "UnityEngine", "Camera", "get_worldToCameraMatrix_Injected", 1);
    g_GetProjectionMatrix = (GetMatrix_t)ResolveMethodPtr(imgCore, "UnityEngine", "Camera", "get_projectionMatrix_Injected", 1);
    g_GetFOV = (GetFOV_t)ResolveMethodPtr(imgCore, "UnityEngine", "Camera", "get_fieldOfView", 0);
    g_SetFOV = (SetFOV_t)ResolveMethodPtr(imgCore, "UnityEngine", "Camera", "set_fieldOfView", 1);

    // Game function pointers
    g_GetMainCameraComDirect = (GetMainCameraComDirect_t)ResolveMethodPtr(imgCSharp, "", "CameraManager", "get_MainCameraCom", 0);
    g_GetWarCash = (GetWarCash_t)ResolveMethodPtr(imgCSharp, "", "PlayerProp", "get_WarCash", 0);
    g_SetWarCash = (SetWarCash_t)ResolveMethodPtr(imgCSharp, "", "PlayerProp", "set_WarCash", 1);
    g_GetPlayerPropFunc = (GetPlayerProp_t)ResolveMethodPtr(imgCSharp, "", "NewObjectCache", "GetPlayerProp", 1);

    // --- Hook target addresses (resolved here, used by InstallHooks) ---

    // Aimbot
    g_EnableCtrlAddr = ResolveMethodPtr(imgCSharp, "SkillBolt", "RayCastCartoon", "EnableCtrl", 16);
    g_EnableAddr = ResolveMethodPtr(imgCSharp, "SkillBolt", "RayCastCartoon", "Enable", 14);
    // ThrowEnableCtrl: cannot identify correct overload by name alone, disable hook
    g_ThrowEnableCtrlAddr = nullptr;
    printf("[GFR Mod] ThrowEnableCtrl: skill aimbot disabled (method unresolvable by name)\n");

    // NoRecoil
    g_CameraCtrlRecoilAddr = ResolveMethodPtr(imgCSharp, "", "CameraCtrl", "Recoil", 0);
    g_SightLogicRecoilAddr = ResolveMethodPtr(imgCSharp, "", "Sight_logic", "Recoil", 0);
    g_SightLogicBulletRecoilAddr = ResolveMethodPtr(imgCSharp, "", "Sight_logic", "BulletRecoil", 0);
    g_WeaponMotionCtrlApplyRecoilAddr = ResolveMethodPtr(imgCSharp, "", "WeaponMotionCtrl", "ApplyRecoil", 0);

    // NoSpread - resolve SightData getters (functional equivalent)
    g_GetCurDisAddr = ResolveMethodPtr(imgCSharp, "", "SightData", "get_CurPosDis", 0);
    g_GetCurBulletTraceRadiusAddr = ResolveMethodPtr(imgCSharp, "", "SightData", "get_BulletTrackRadius", 0);

    // FOV
    g_GetFOVAddr = ResolveMethodPtr(imgCore, "UnityEngine", "Camera", "get_fieldOfView", 0);

    // Weakness hit hack
    g_CartoonDataSetSkilllRayAddr = ResolveMethodPtr(imgCSharp, "SkillBolt", "CartoonData", "SetSkilllRay", 1);
    g_CartoonDataPacketSkillRayAddr = ResolveMethodPtr(imgCSharp, "SkillBolt", "CartoonData", "PacketSkillRay", 2);
    g_SClientHitInfoCtorAddr = ResolveMethodPtr(imgCSharp, "SkillBolt", "SClientHitInfo", ".ctor", 10);

    // Log all resolved addresses for debugging
    printf("[GFR Mod] === Method Resolution Results ===\n");
    printf("[GFR Mod] GameAssembly base: %p\n", g_GameAssembly);
    printf("[GFR Mod] il2cpp_method_get_pointer: %s\n", il2cpp_method_get_pointer ? "available" : "NOT available (using fallback)");
    printf("[GFR Mod] EnableCtrl:       %p\n", g_EnableCtrlAddr);
    printf("[GFR Mod] Enable:           %p\n", g_EnableAddr);
    printf("[GFR Mod] ThrowEnableCtrl:  %p\n", g_ThrowEnableCtrlAddr);
    printf("[GFR Mod] CameraRecoil:     %p\n", g_CameraCtrlRecoilAddr);
    printf("[GFR Mod] SightRecoil:      %p\n", g_SightLogicRecoilAddr);
    printf("[GFR Mod] SightBulletRecoil:%p\n", g_SightLogicBulletRecoilAddr);
    printf("[GFR Mod] WeaponRecoil:     %p\n", g_WeaponMotionCtrlApplyRecoilAddr);
    printf("[GFR Mod] GetCurDis:        %p\n", g_GetCurDisAddr);
    printf("[GFR Mod] GetBulletRadius:  %p\n", g_GetCurBulletTraceRadiusAddr);
    printf("[GFR Mod] GetFOV:           %p\n", g_GetFOVAddr);
    printf("[GFR Mod] GetPosInjected:   %p\n", g_GetPositionInjected);
    printf("[GFR Mod] SetPosInjected:   %p\n", g_SetPositionInjected);
    printf("[GFR Mod] WorldToCamMatrix: %p\n", g_GetWorldToCameraMatrix);
    printf("[GFR Mod] ProjMatrix:       %p\n", g_GetProjectionMatrix);
    printf("[GFR Mod] GetWarCash:       %p\n", g_GetWarCash);
    printf("[GFR Mod] SetWarCash:       %p\n", g_SetWarCash);
    printf("[GFR Mod] MainCameraCom:    %p\n", g_GetMainCameraComDirect);
    printf("[GFR Mod] WeaknessSetRay:   %p\n", g_CartoonDataSetSkilllRayAddr);
    printf("[GFR Mod] WeaknessPacket:   %p\n", g_CartoonDataPacketSkillRayAddr);
    printf("[GFR Mod] HitInfoCtor:      %p\n", g_SClientHitInfoCtorAddr);
    printf("[GFR Mod] ================================\n");

    // Create weakness strings using il2cpp_string_new
    if (il2cpp_string_new) {
        g_WeaknessString = il2cpp_string_new("Monster_Weakness");
        g_SpecialWeaknessString = il2cpp_string_new("Monster_SpecialWeakness");
    }

    return g_GetMonsters && g_GetWeakTrans && g_GetPosition;
}

Il2CppObject* GetLocalPlayer() {
    if (!g_NewPlayerManager || !g_GetPlayer || !il2cpp_field_static_get_value || !g_MainCtrlField)
        return nullptr;
    
    int mainCtrl = 0;
    il2cpp_field_static_get_value(g_MainCtrlField, &mainCtrl);
    if (mainCtrl == 0) return nullptr;
    
    void* args[] = { &mainCtrl };
    return il2cpp_runtime_invoke(g_GetPlayer, nullptr, args, nullptr);
}

Il2CppObject* GetCurrentWeaponItemProp(Il2CppObject* localPlayer) {
    if (!localPlayer) return nullptr;
    
    auto playerCom = *(Il2CppObject**)((char*)localPlayer + OFFSET_PLAYERCOM);
    if (!playerCom) return nullptr;
    
    auto curWeaponSlot = *(Il2CppObject**)((char*)playerCom + OFFSET_CURWEAPONASSLOT);
    if (!curWeaponSlot) return nullptr;
    
    auto fstASObj = *(Il2CppObject**)((char*)curWeaponSlot + OFFSET_FSTASOBJ);
    if (!fstASObj) return nullptr;
    
    auto reloadCom = *(Il2CppObject**)((char*)fstASObj + OFFSET_RELOADCOM);
    if (!reloadCom) return nullptr;
    
    return *(Il2CppObject**)((char*)reloadCom + OFFSET_ITEMPROP);
}

void RefillAmmo(Il2CppObject* itemProp) {
    if (!itemProp || !g_GetMaxBullet || !g_GetCurBullet) return;

    auto maxBulletObj = il2cpp_runtime_invoke(g_GetMaxBullet, itemProp, nullptr, nullptr);
    if (!maxBulletObj) return;
    int maxBullet = *(int*)il2cpp_object_unbox(maxBulletObj);

    auto curBulletObj = il2cpp_runtime_invoke(g_GetCurBullet, itemProp, nullptr, nullptr);
    if (curBulletObj) {
        int curBullet = *(int*)il2cpp_object_unbox(curBulletObj);
        if (curBullet >= maxBullet) return;
    }

    void* args[] = { &maxBullet };
    if (g_SetCurBullet)
        il2cpp_runtime_invoke(g_SetCurBullet, itemProp, args, nullptr);
    if (g_SetClientCurBullet)
        il2cpp_runtime_invoke(g_SetClientCurBullet, itemProp, args, nullptr);
}

void SetPlayerSpeed(Il2CppObject* localPlayer, int speed) {
    if (!localPlayer || !g_SetSpeed) return;
    
    auto playerProp = *(Il2CppObject**)((char*)localPlayer + OFFSET_PLAYERPROP);
    if (!playerProp) return;
    
    void* args[] = { &speed };
    il2cpp_runtime_invoke(g_SetSpeed, playerProp, args, nullptr);
}

int GetPlayerSpeed(Il2CppObject* localPlayer) {
    if (!localPlayer || !g_GetSpeed) return 500;
    
    auto playerProp = *(Il2CppObject**)((char*)localPlayer + OFFSET_PLAYERPROP);
    if (!playerProp) return 500;
    
    auto result = il2cpp_runtime_invoke(g_GetSpeed, playerProp, nullptr, nullptr);
    if (result) return *(int*)il2cpp_object_unbox(result);
    return 500;
}

void SetJumpHeight(Il2CppObject* localPlayer, float height) {
    if (!localPlayer || !g_SetJumpHeight) return;
    
    auto playerProp = *(Il2CppObject**)((char*)localPlayer + OFFSET_PLAYERPROP);
    if (!playerProp) return;
    
    void* args[] = { &height };
    il2cpp_runtime_invoke(g_SetJumpHeight, playerProp, args, nullptr);
}

float GetJumpHeight(Il2CppObject* localPlayer) {
    if (!localPlayer || !g_GetJumpHeight) return 1.0f;
    
    auto playerProp = *(Il2CppObject**)((char*)localPlayer + OFFSET_PLAYERPROP);
    if (!playerProp) return 1.0f;
    
    auto result = il2cpp_runtime_invoke(g_GetJumpHeight, playerProp, nullptr, nullptr);
    if (result) return *(float*)il2cpp_object_unbox(result);
    return 1.0f;
}

void AutoPickup() {
    if (!g_PlayerDictField || !g_MainCtrlField || !g_GetPositionInjected || !g_SetPositionInjected)
        return;
    
    int mainCtrl = 0;
    il2cpp_field_static_get_value(g_MainCtrlField, &mainCtrl);
    if (mainCtrl == 0) return;
    
    Il2CppObject* playerDict = nullptr;
    il2cpp_field_static_get_value(g_PlayerDictField, &playerDict);
    if (!playerDict) return;
    
    auto entries = *(Il2CppObject**)((char*)playerDict + 0x18);
    int dictCount = *(int*)((char*)playerDict + 0x20);
    if (!entries || dictCount <= 0) return;
    
    Il2CppObject* playerTrans = nullptr;
    for (int i = 0; i < dictCount + 50; i++) {
        char* entryBase = (char*)entries + 0x20 + i * 24;
        int hashCode = *(int*)entryBase;
        if (hashCode < 0) continue;
        
        int key = *(int*)(entryBase + 0x8);
        if (key != mainCtrl) continue;
        
        auto playerObj = *(Il2CppObject**)(entryBase + 0x10);
        if (!playerObj) continue;
        
        playerTrans = *(Il2CppObject**)((char*)playerObj + OFFSET_GAMETRANS);
        break;
    }
    
    if (!playerTrans) return;
    
    Vector3 playerPos;
    g_GetPositionInjected(playerTrans, &playerPos);
    Vector3 targetPos = { playerPos.x, playerPos.y + 0.5f, playerPos.z };
    
    for (int i = 0; i < dictCount + 50; i++) {
        char* entryBase = (char*)entries + 0x20 + i * 24;
        int hashCode = *(int*)entryBase;
        if (hashCode < 0) continue;
        
        auto playerObj = *(Il2CppObject**)(entryBase + 0x10);
        if (!playerObj) continue;
        
        auto dropCom = *(Il2CppObject**)((char*)playerObj + OFFSET_DROPOPCOM);
        if (!dropCom) continue;
        
        auto gameTrans = *(Il2CppObject**)((char*)playerObj + OFFSET_GAMETRANS);
        if (!gameTrans) continue;
        
        g_SetPositionInjected(gameTrans, &targetPos);
    }
}


// Get best weakness transform (SpecialWeakness > Monster_Weakness)
void* GetBestWeaknessTrans(void* bodyPartCom) {
    if (!bodyPartCom) return nullptr;
    
    // 1순위: SpecialWeakness
    if (g_GetSpecialWeakTrans) {
        auto specialTrans = il2cpp_runtime_invoke(g_GetSpecialWeakTrans, bodyPartCom, nullptr, nullptr);
        if (specialTrans) return specialTrans;
    }
    
    // 2순위: Monster_Weakness (GetWeakTrans with findNearest=true)
    if (g_GetWeakTrans) {
        bool findNearest = true;
        void* args[] = { &findNearest };
        auto weakTrans = il2cpp_runtime_invoke(g_GetWeakTrans, bodyPartCom, args, nullptr);
        if (weakTrans) return weakTrans;
    }
    
    return nullptr;
}

bool HookedGetNoCostBullet(void* thisPtr, const void* method) {
    if (g_InfiniteAmmo) {
        return true;
    }
    return g_OriginalGetNoCostBullet ? g_OriginalGetNoCostBullet(thisPtr, method) : false;
}
