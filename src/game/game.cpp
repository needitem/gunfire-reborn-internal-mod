#include "game.h"
#include "../features/norecoil.h"
#include "../features/aimbot.h"
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

// Infinite ammo hook
void* g_GetNoCostBulletAddr = nullptr;
GetNoCostBullet_t g_OriginalGetNoCostBullet = nullptr;


// No Cooldown hook
void* g_ReturnNocdAddr = nullptr;
ReturnNocd_t g_OriginalReturnNocd = nullptr;

// Weakness hit hack
CartoonDataSetSkilllRay_t g_OriginalCartoonDataSetSkilllRay = nullptr;
CartoonDataPacketSkillRay_t g_OriginalCartoonDataPacketSkillRay = nullptr;
SClientHitInfoCtor_t g_OriginalSClientHitInfoCtor = nullptr;

void* g_CartoonDataSetSkilllRayAddr = nullptr;
void* g_CartoonDataPacketSkillRayAddr = nullptr;
void* g_SClientHitInfoCtorAddr = nullptr;

void* g_WeaknessString = nullptr;
void* g_SpecialWeaknessString = nullptr;

OnReloadBullet_t g_OriginalOnReloadBullet = nullptr;
void* g_OnReloadBulletAddr = nullptr;

OnReloadBulletCallBack_t g_OnReloadBulletCallBack = nullptr;
void* g_OnReloadBulletCallBackAddr = nullptr;

// ========== COMPREHENSIVE METADATA DUMPER ==========
// Dumps ALL classes, methods, and fields from Assembly-CSharp to a file

static void DumpClassToFile(FILE* f, Il2CppClass* klass, int depth) {
    if (!klass || !f) return;

    const char* className = il2cpp_class_get_name ? il2cpp_class_get_name(klass) : "???";
    const char* ns = il2cpp_class_get_namespace ? il2cpp_class_get_namespace(klass) : "";

    // Print class header with namespace
    if (ns && ns[0])
        fprintf(f, "%*sclass %s.%s", depth * 2, "", ns, className);
    else
        fprintf(f, "%*sclass %s", depth * 2, "", className);

    // Print parent class
    if (il2cpp_class_get_parent) {
        Il2CppClass* parent = il2cpp_class_get_parent(klass);
        if (parent && parent != klass) {
            const char* parentName = il2cpp_class_get_name ? il2cpp_class_get_name(parent) : "???";
            fprintf(f, " : %s", parentName);
        }
    }
    fprintf(f, " {\n");

    // Dump fields
    if (il2cpp_class_get_fields && il2cpp_field_get_name) {
        void* fiter = nullptr;
        void* field;
        while ((field = il2cpp_class_get_fields(klass, &fiter)) != nullptr) {
            const char* fname = il2cpp_field_get_name(field);
            if (!fname) continue;
            const char* ftypeName = "?";
            if (il2cpp_field_get_type && il2cpp_type_get_name) {
                void* ftype = il2cpp_field_get_type(field);
                if (ftype) {
                    const char* tn = il2cpp_type_get_name(ftype);
                    if (tn) ftypeName = tn;
                }
            }
            size_t offset = il2cpp_field_get_offset ? il2cpp_field_get_offset(field) : 0;
            fprintf(f, "%*s  [0x%zx] %s %s;\n", depth * 2, "", offset, ftypeName, fname);
        }
    }

    // Dump methods
    if (il2cpp_class_get_methods && il2cpp_method_get_name) {
        void* miter = nullptr;
        void* method;
        while ((method = il2cpp_class_get_methods(klass, &miter)) != nullptr) {
            const char* mname = il2cpp_method_get_name(method);
            if (!mname) continue;

            // Return type
            const char* retTypeName = "void";
            if (il2cpp_method_get_return_type && il2cpp_type_get_name) {
                void* retType = il2cpp_method_get_return_type(method);
                if (retType) {
                    const char* rtn = il2cpp_type_get_name(retType);
                    if (rtn) retTypeName = rtn;
                }
            }

            int paramCount = il2cpp_method_get_param_count ? il2cpp_method_get_param_count(method) : 0;

            // Method pointer
            void* ptr = nullptr;
            if (il2cpp_method_get_pointer) ptr = il2cpp_method_get_pointer((Il2CppMethod*)method);
            if (!ptr) ptr = *(void**)method;

            fprintf(f, "%*s  %s %s(", depth * 2, "", retTypeName, mname);

            // Parameter types
            if (il2cpp_method_get_param && il2cpp_type_get_name && paramCount > 0) {
                for (int p = 0; p < paramCount; p++) {
                    void* paramType = il2cpp_method_get_param(method, p);
                    const char* ptn = "?";
                    if (paramType) {
                        const char* tn = il2cpp_type_get_name(paramType);
                        if (tn) ptn = tn;
                    }
                    if (p > 0) fprintf(f, ", ");
                    fprintf(f, "%s", ptn);
                }
            } else if (paramCount > 0) {
                fprintf(f, "%d params", paramCount);
            }

            fprintf(f, ") // RVA: 0x%llx\n", ptr ? (unsigned long long)((uintptr_t)ptr - (uintptr_t)g_GameAssembly) : 0ULL);
        }
    }

    fprintf(f, "%*s}\n\n", depth * 2, "");
}

static void DumpAllMetadata() {
    if (!il2cpp_image_get_class_count || !il2cpp_image_get_class) {
        printf("[GFR Mod] Metadata dump: missing il2cpp_image_get_class_count/get_class APIs\n");
        return;
    }

    auto imgCSharp = FindImage("Assembly-CSharp");
    if (!imgCSharp) {
        printf("[GFR Mod] Metadata dump: Assembly-CSharp not found\n");
        return;
    }

    // Write dump to game directory
    const char* dumpPath = "gfr_metadata_dump.txt";
    FILE* f = fopen(dumpPath, "w");
    if (!f) {
        // Try desktop
        dumpPath = "C:\\Users\\th072\\Desktop\\gfr_metadata_dump.txt";
        f = fopen(dumpPath, "w");
    }
    if (!f) {
        printf("[GFR Mod] Metadata dump: failed to open output file\n");
        return;
    }

    size_t classCount = il2cpp_image_get_class_count(imgCSharp);
    printf("[GFR Mod] Metadata dump: %zu classes in Assembly-CSharp\n", classCount);
    fprintf(f, "// GFR Metadata Dump - Assembly-CSharp\n");
    fprintf(f, "// Total classes: %zu\n\n", classCount);

    for (size_t i = 0; i < classCount; i++) {
        __try {
            Il2CppClass* klass = il2cpp_image_get_class(imgCSharp, i);
            if (!klass) continue;
            DumpClassToFile(f, klass, 0);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            fprintf(f, "// [ERROR] Exception dumping class index %zu\n\n", i);
        }
    }

    fclose(f);
    printf("[GFR Mod] Metadata dump complete: %s (%zu classes)\n", dumpPath, classCount);
}

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

    // Reload completion hook (used by infinite ammo)
    g_OnReloadBulletAddr = ResolveMethodPtr(imgCSharp, "", "HeroAttackCtrl", "OnReloadBullet", 2);
    g_OnReloadBulletCallBackAddr = ResolveMethodPtr(imgCSharp, "", "HeroAttackCtrl", "OnReloadBulletCallBack", 2);
    g_OnReloadBulletCallBack = (OnReloadBulletCallBack_t)g_OnReloadBulletCallBackAddr;
    printf("[GFR Mod] Reload: OnReloadBullet=%p, Callback=%p\n", g_OnReloadBulletAddr, g_OnReloadBulletCallBackAddr);

    if (g_DropManager) {
        g_OneGPU = il2cpp_class_get_method_from_name(g_DropManager, "OneGPU", 0);
        g_StartGPU = il2cpp_class_get_method_from_name(g_DropManager, "StartGPU", 1);
    }
    
    if (g_OCBodyPart) {
        g_GetWeakTrans = il2cpp_class_get_method_from_name(g_OCBodyPart, "GetWeakTrans", 1);
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

    // GMStateManager - hook GetNoCostBullet for infinite ammo, returnNocd for no cooldown
    g_GMStateManager = il2cpp_class_from_name(imgCSharp, "", "GMStateManager");
    if (g_GMStateManager) {
        g_GetNoCostBulletAddr = ResolveMethodPtr(imgCSharp, "", "GMStateManager", "GetNoCostBullet", 0);
        g_ReturnNocdAddr = ResolveMethodPtr(imgCSharp, "", "GMStateManager", "returnNocd", 0);
    }

    // --- Resolve all method pointers at runtime via IL2CPP API (no hardcoded RVAs) ---
    auto imgPhysics = FindImage("UnityEngine.PhysicsModule");

    // Unity engine function pointers
    g_GetPositionInjected = (GetPositionInjected_t)ResolveMethodPtr(imgCore, "UnityEngine", "Transform", "get_position_Injected", 1);
    g_SetPositionInjected = (SetPositionInjected_t)ResolveMethodPtr(imgCore, "UnityEngine", "Transform", "set_position_Injected", 1);
    g_GetWorldToCameraMatrix = (GetMatrix_t)ResolveMethodPtr(imgCore, "UnityEngine", "Camera", "get_worldToCameraMatrix_Injected", 1);
    g_GetProjectionMatrix = (GetMatrix_t)ResolveMethodPtr(imgCore, "UnityEngine", "Camera", "get_projectionMatrix_Injected", 1);

    // Game function pointers
    g_GetMainCameraComDirect = (GetMainCameraComDirect_t)ResolveMethodPtr(imgCSharp, "", "CameraManager", "get_MainCameraCom", 0);
    g_GetWarCash = (GetWarCash_t)ResolveMethodPtr(imgCSharp, "", "PlayerProp", "get_WarCash", 0);
    g_SetWarCash = (SetWarCash_t)ResolveMethodPtr(imgCSharp, "", "PlayerProp", "set_WarCash", 1);
    g_GetPlayerPropFunc = (GetPlayerProp_t)ResolveMethodPtr(imgCSharp, "", "NewObjectCache", "GetPlayerProp", 1);

    // --- Hook target addresses (resolved here, used by InstallHooks) ---

    // Aimbot
    g_EnableCtrlAddr = ResolveMethodPtr(imgCSharp, "SkillBolt", "RayCastCartoon", "EnableCtrl", 16);
    g_EnableAddr = ResolveMethodPtr(imgCSharp, "SkillBolt", "RayCastCartoon", "Enable", 14);
    // Skill aim hooks are temporarily disabled.
    g_ThrowEnableCtrlAddr = nullptr;
    g_ParabolaEnableCtrlAddr = nullptr;

    // NoRecoil (Sight_logic.Recoil and BulletRecoil removed in game update)
    g_CameraCtrlRecoilAddr = ResolveMethodPtr(imgCSharp, "", "CameraCtrl", "Recoil", 0);
    g_WeaponMotionCtrlApplyRecoilAddr = ResolveMethodPtr(imgCSharp, "", "WeaponMotionCtrl", "ApplyRecoil", 0);

    // NoSpread - resolve SightData getters (functional equivalent)
    g_GetCurDisAddr = ResolveMethodPtr(imgCSharp, "", "SightData", "get_CurPosDis", 0);
    g_GetCurBulletTraceRadiusAddr = ResolveMethodPtr(imgCSharp, "", "SightData", "get_BulletTrackRadius", 0);

    // Weakness hit hack
    g_CartoonDataSetSkilllRayAddr = ResolveMethodPtr(imgCSharp, "SkillBolt", "CartoonData", "SetSkilllRay", 1);
    g_CartoonDataPacketSkillRayAddr = ResolveMethodPtr(imgCSharp, "SkillBolt", "CartoonData", "PacketSkillRay", 2);
    g_SClientHitInfoCtorAddr = ResolveMethodPtr(imgCSharp, "SkillBolt", "SClientHitInfo", ".ctor", 10);

    // Create weakness strings using il2cpp_string_new
    if (il2cpp_string_new) {
        g_WeaknessString = il2cpp_string_new("Monster_Weakness");
        g_SpecialWeaknessString = il2cpp_string_new("Monster_SpecialWeakness");
    }

    // Metadata dump disabled (already completed ??see gfr_metadata_dump.txt)
    // DumpAllMetadata();

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

    if (g_ShuttingDown.load(std::memory_order_acquire)) return;

    __try {
        int mainCtrl = 0;
        il2cpp_field_static_get_value(g_MainCtrlField, &mainCtrl);
        if (mainCtrl == 0) return;

        Il2CppObject* playerDict = nullptr;
        il2cpp_field_static_get_value(g_PlayerDictField, &playerDict);
        if (!playerDict) return;

        auto entries = *(Il2CppObject**)((char*)playerDict + 0x18);
        int dictCount = *(int*)((char*)playerDict + 0x20);
        if (!entries || dictCount <= 0 || dictCount > 500) return;

        int scanCount = dictCount + 50;
        if (scanCount > 500) scanCount = 500;

        Il2CppObject* playerTrans = nullptr;
        for (int i = 0; i < scanCount; i++) {
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

        for (int i = 0; i < scanCount; i++) {
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
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        // Scene transition can invalidate object pointers for a few frames.
    }
}


// Get best weakness transform (Monster_Weakness with findNearest=true)
void* GetBestWeaknessTrans(void* bodyPartCom) {
    if (!bodyPartCom) return nullptr;

    // GetWeakTrans with findNearest=true
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

bool HookedReturnNocd(void* thisPtr, const void* method) {
    if (g_NoCooldown) {
        return true;
    }
    return g_OriginalReturnNocd ? g_OriginalReturnNocd(thisPtr, method) : false;
}

// HeroAttackCtrl.OnReloadBullet - integrated into infinite ammo
bool HookedOnReloadBullet(void* thisPtr, int weaponid, int reason) {
    if (g_InfiniteAmmo) {
        // Call callback immediately to complete reload.
        if (g_OnReloadBulletCallBack) {
            g_OnReloadBulletCallBack(thisPtr, weaponid, 1);
        }
        return true;
    }
    return g_OriginalOnReloadBullet ? g_OriginalOnReloadBullet(thisPtr, weaponid, reason) : false;
}
