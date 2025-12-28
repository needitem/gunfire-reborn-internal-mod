#define _USE_MATH_DEFINES
#include <Windows.h>
#include <cstdio>
#include <cmath>
#include <thread>
#include <string>
#include <atomic>
#include <MinHook.h>

// IL2CPP types
typedef void* Il2CppObject;
typedef void* Il2CppClass;
typedef void* Il2CppMethod;
typedef void* Il2CppDomain;
typedef void* Il2CppAssembly;
typedef void* Il2CppImage;

struct Vector3 { float x, y, z; };

// IL2CPP function pointers
typedef Il2CppDomain* (*il2cpp_domain_get_t)();
typedef Il2CppAssembly** (*il2cpp_domain_get_assemblies_t)(Il2CppDomain*, size_t*);
typedef Il2CppImage* (*il2cpp_assembly_get_image_t)(Il2CppAssembly*);
typedef const char* (*il2cpp_image_get_name_t)(Il2CppImage*);
typedef Il2CppClass* (*il2cpp_class_from_name_t)(Il2CppImage*, const char*, const char*);
typedef Il2CppMethod* (*il2cpp_class_get_method_from_name_t)(Il2CppClass*, const char*, int);
typedef Il2CppObject* (*il2cpp_runtime_invoke_t)(Il2CppMethod*, void*, void**, void**);
typedef void* (*il2cpp_object_unbox_t)(Il2CppObject*);
typedef void* (*il2cpp_class_get_field_from_name_t)(Il2CppClass*, const char*);
typedef void (*il2cpp_field_static_get_value_t)(void*, void*);
typedef void (*il2cpp_thread_attach_t)(Il2CppDomain*);
typedef void (*il2cpp_thread_detach_t)(void*);
typedef void* (*il2cpp_method_get_pointer_t)(Il2CppMethod*);
typedef Il2CppObject* (*il2cpp_string_new_t)(const char*);

// Global function pointers
il2cpp_domain_get_t il2cpp_domain_get;
il2cpp_domain_get_assemblies_t il2cpp_domain_get_assemblies;
il2cpp_assembly_get_image_t il2cpp_assembly_get_image;
il2cpp_image_get_name_t il2cpp_image_get_name;
il2cpp_class_from_name_t il2cpp_class_from_name;
il2cpp_class_get_method_from_name_t il2cpp_class_get_method_from_name;
il2cpp_runtime_invoke_t il2cpp_runtime_invoke;
il2cpp_object_unbox_t il2cpp_object_unbox;
il2cpp_class_get_field_from_name_t il2cpp_class_get_field_from_name;
il2cpp_field_static_get_value_t il2cpp_field_static_get_value;
il2cpp_thread_attach_t il2cpp_thread_attach;
il2cpp_thread_detach_t il2cpp_thread_detach;
il2cpp_method_get_pointer_t il2cpp_method_get_pointer;
il2cpp_string_new_t il2cpp_string_new;

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

// Cached field/class
void* g_MainCtrlField = nullptr;
Il2CppClass* g_NewPlayerManager = nullptr;
Il2CppClass* g_NewItemProp = nullptr;
Il2CppClass* g_PlayerProp = nullptr;
Il2CppClass* g_DropManager = nullptr;
Il2CppClass* g_OCBodyPart = nullptr;

// Offsets
const size_t OFFSET_PLAYERCOM = 0x128;
const size_t OFFSET_CURWEAPONASSLOT = 0x68;
const size_t OFFSET_FSTASOBJ = 0x50;
const size_t OFFSET_RELOADCOM = 0x10;
const size_t OFFSET_ITEMPROP = 0x18;
const size_t OFFSET_PLAYERPROP = 0x48;
const size_t OFFSET_BODYPARTCOM = 0x160;
const size_t OFFSET_GAMETRANS = 0x88;
const size_t OFFSET_DROPOPCOM = 0xd0;

// RVAs for hooking
const DWORD RVA_RAYCAST_ENABLE_CTRL = 0x01364c90;
const DWORD RVA_RAYCAST_ENABLE = 0x01365450;
const DWORD RVA_GET_POSITION_INJECTED = 0x03783270;
const DWORD RVA_SET_POSITION_INJECTED = 0x03783eb0;

// Physics.Linecast for visibility check
const DWORD RVA_PHYSICS_LINECAST = 0x036730d0;

// NOTE: TraceCartoon (prism/shuriken) and CastingRayCartoon (laser) use server-side hit detection
// Client-side aimbot is not possible for these weapon types

// ThrowByPowerCartoon RVAs (for grenade/skill aimbot)
const DWORD RVA_THROW_ENABLE_CTRL = 0x014f0fb0;
const DWORD RVA_THROW_ENABLE = 0x014f1de0;

// NoRecoil RVAs (direct from dump)
const DWORD RVA_CAMERACTRL_RECOIL = 0x00d8fc90;
const DWORD RVA_SIGHTLOGIC_RECOIL = 0x00a7ebf0;
const DWORD RVA_SIGHTLOGIC_BULLETRECOIL = 0x00a7b7a0;
const DWORD RVA_WEAPONMOTIONCTRL_APPLYRECOIL = 0x00fff630;

// NoSpread RVAs
const DWORD RVA_SIGHTLOGIC_GETCURDIS = 0x00a7c3b0;
const DWORD RVA_SIGHTLOGIC_GETCURBULLETTRACERADIUS = 0x00a7c380;

// FOV RVAs
const DWORD RVA_CAMERA_GET_FOV = 0x025d1480;
const DWORD RVA_CAMERA_SET_FOV = 0x025d22c0;

// Transform position functions
typedef void (*GetPositionInjected_t)(void* transform, Vector3* ret);
typedef void (*SetPositionInjected_t)(void* transform, Vector3* value);
GetPositionInjected_t g_GetPositionInjected = nullptr;
SetPositionInjected_t g_SetPositionInjected = nullptr;

// PlayerDict field
void* g_PlayerDictField = nullptr;

// NoRecoil/NoSpread methods (will be resolved at runtime)
Il2CppMethod* g_CameraCtrl_Recoil = nullptr;
Il2CppMethod* g_SightLogic_Recoil = nullptr;
Il2CppMethod* g_SightLogic_BulletRecoil = nullptr;
Il2CppMethod* g_WeaponMotionCtrl_ApplyRecoil = nullptr;
Il2CppMethod* g_GetAccuracy = nullptr;
Il2CppMethod* g_GetStability = nullptr;
Il2CppMethod* g_GetCurBulletHook = nullptr;
Il2CppClass* g_CameraCtrl = nullptr;
Il2CppClass* g_SightLogic = nullptr;
Il2CppClass* g_WeaponMotionCtrl = nullptr;
Il2CppClass* g_GMStateManager = nullptr;
Il2CppMethod* g_SetNoCostBullet = nullptr;

// FOV function pointers
typedef float (*GetFOV_t)(void* camera);
typedef void (*SetFOV_t)(void* camera, float fov);
GetFOV_t g_GetFOV = nullptr;
SetFOV_t g_SetFOV = nullptr;
void* g_CachedCamera = nullptr;

// Settings
bool g_Running = true;
bool g_SilentAimEnabled = true;
bool g_SkillAimEnabled = false;  // Skill aimbot (grenade, etc.) - OFF by default
bool g_InfiniteAmmo = true;
bool g_SpeedBoost = true;
bool g_NoRecoil = true;
bool g_NoSpread = true;
bool g_FastBullet = true;
bool g_FOVEnabled = false;  // FOV hack - OFF by default
float g_CustomFOV = 120.0f;  // Custom FOV value
float g_OriginalFOV = 0.0f;  // Store original FOV
float g_BulletSpeedMultiplier = 100.0f;
float g_SkillSpeedMultiplier = 10.0f;  // Skill projectile speed multiplier
int g_OriginalSpeed = 0;
int g_BoostedSpeed = 1000;
float g_OriginalJumpHeight = 0.0f;
float g_BoostedJumpHeight = 1.3f;  // 1.3x jump height
DWORD g_LastSpeedCheck = 0;
DWORD g_LastAmmoRefill = 0;

// Cached target for silent aim (updated in main loop, used in hooks)
// Using SRWLOCK for proper thread synchronization
struct CachedTarget {
    float x;
    float y;
    float z;
    bool valid;
};
CachedTarget g_CachedTarget = {0, 0, 0, false};
SRWLOCK g_TargetLock = SRWLOCK_INIT;

// Atomic flag for safe shutdown
std::atomic<bool> g_ShuttingDown(false);
std::atomic<bool> g_HooksInstalled(false);

HMODULE g_GameAssembly = nullptr;

// Hook function types
typedef void (*EnableCtrl_t)(void*, void*, Vector3*, Vector3*, Vector3*, int, float, float, int, void*, float, void*, float, float, float, int, int);
typedef void (*Enable_t)(void*, void*, Vector3*, Vector3*, Vector3*, int, float, float, int, void*, float, void*, float, float, float);
typedef void (*RecoilFunc_t)(void*);
typedef float (*GetFloatFunc_t)(void*);

// ThrowByPowerCartoon function types (for skill aimbot)
// EnableCtrl(skill, start, dir, speed, radius, upForce, bounciness, delayTrigger, hitOver, hitStaticOver, ...)
typedef void (*ThrowEnableCtrl_t)(void*, void*, Vector3*, Vector3*, float, float, Vector3*, void*, int, bool, bool, void*, float, void*, float, float, int, bool, void*, int, int, float, float, bool, int, bool, float, float, bool, float, float);

EnableCtrl_t g_OriginalEnableCtrl = nullptr;
Enable_t g_OriginalEnable = nullptr;
ThrowEnableCtrl_t g_OriginalThrowEnableCtrl = nullptr;
RecoilFunc_t g_OriginalCameraCtrlRecoil = nullptr;
RecoilFunc_t g_OriginalSightLogicRecoil = nullptr;
RecoilFunc_t g_OriginalSightLogicBulletRecoil = nullptr;
RecoilFunc_t g_OriginalWeaponMotionCtrlApplyRecoil = nullptr;
GetFloatFunc_t g_OriginalGetAccuracy = nullptr;
GetFloatFunc_t g_OriginalGetStability = nullptr;
GetFloatFunc_t g_OriginalGetCurDis = nullptr;
GetFloatFunc_t g_OriginalGetCurBulletTraceRadius = nullptr;
typedef int (*GetIntFunc_t)(void*);
GetIntFunc_t g_OriginalGetCurBullet = nullptr;

Il2CppImage* FindImage(const char* name) {
    auto domain = il2cpp_domain_get();
    if (!domain) return nullptr;
    
    size_t count = 0;
    auto assemblies = il2cpp_domain_get_assemblies(domain, &count);
    
    for (size_t i = 0; i < count; i++) {
        auto img = il2cpp_assembly_get_image(assemblies[i]);
        auto imgName = il2cpp_image_get_name(img);
        if (imgName && strstr(imgName, name)) return img;
    }
    return nullptr;
}

bool InitIL2CPP() {
    g_GameAssembly = GetModuleHandleA("GameAssembly.dll");
    if (!g_GameAssembly) return false;
    
    il2cpp_domain_get = (il2cpp_domain_get_t)GetProcAddress(g_GameAssembly, "il2cpp_domain_get");
    il2cpp_domain_get_assemblies = (il2cpp_domain_get_assemblies_t)GetProcAddress(g_GameAssembly, "il2cpp_domain_get_assemblies");
    il2cpp_assembly_get_image = (il2cpp_assembly_get_image_t)GetProcAddress(g_GameAssembly, "il2cpp_assembly_get_image");
    il2cpp_image_get_name = (il2cpp_image_get_name_t)GetProcAddress(g_GameAssembly, "il2cpp_image_get_name");
    il2cpp_class_from_name = (il2cpp_class_from_name_t)GetProcAddress(g_GameAssembly, "il2cpp_class_from_name");
    il2cpp_class_get_method_from_name = (il2cpp_class_get_method_from_name_t)GetProcAddress(g_GameAssembly, "il2cpp_class_get_method_from_name");
    il2cpp_runtime_invoke = (il2cpp_runtime_invoke_t)GetProcAddress(g_GameAssembly, "il2cpp_runtime_invoke");
    il2cpp_object_unbox = (il2cpp_object_unbox_t)GetProcAddress(g_GameAssembly, "il2cpp_object_unbox");
    il2cpp_class_get_field_from_name = (il2cpp_class_get_field_from_name_t)GetProcAddress(g_GameAssembly, "il2cpp_class_get_field_from_name");
    il2cpp_field_static_get_value = (il2cpp_field_static_get_value_t)GetProcAddress(g_GameAssembly, "il2cpp_field_static_get_value");
    il2cpp_thread_attach = (il2cpp_thread_attach_t)GetProcAddress(g_GameAssembly, "il2cpp_thread_attach");
    il2cpp_thread_detach = (il2cpp_thread_detach_t)GetProcAddress(g_GameAssembly, "il2cpp_thread_detach");
    il2cpp_method_get_pointer = (il2cpp_method_get_pointer_t)GetProcAddress(g_GameAssembly, "il2cpp_method_get_pointer");
    il2cpp_string_new = (il2cpp_string_new_t)GetProcAddress(g_GameAssembly, "il2cpp_string_new");

    if (!il2cpp_domain_get || !il2cpp_runtime_invoke || !il2cpp_thread_attach) return false;
    
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

    // Transform position functions (direct RVA)
    g_GetPositionInjected = (GetPositionInjected_t)((BYTE*)g_GameAssembly + RVA_GET_POSITION_INJECTED);
    g_SetPositionInjected = (SetPositionInjected_t)((BYTE*)g_GameAssembly + RVA_SET_POSITION_INJECTED);

    // NoRecoil/NoSpread - Cache classes and methods
    g_CameraCtrl = il2cpp_class_from_name(imgCSharp, "", "CameraCtrl");
    g_SightLogic = il2cpp_class_from_name(imgCSharp, "", "Sight_logic");
    g_WeaponMotionCtrl = il2cpp_class_from_name(imgCSharp, "", "WeaponMotionCtrl");

    if (g_CameraCtrl) {
        g_CameraCtrl_Recoil = il2cpp_class_get_method_from_name(g_CameraCtrl, "Recoil", 0);
    }
    if (g_SightLogic) {
        g_SightLogic_Recoil = il2cpp_class_get_method_from_name(g_SightLogic, "Recoil", 0);
        g_SightLogic_BulletRecoil = il2cpp_class_get_method_from_name(g_SightLogic, "BulletRecoil", 0);
    }
    if (g_WeaponMotionCtrl) {
        g_WeaponMotionCtrl_ApplyRecoil = il2cpp_class_get_method_from_name(g_WeaponMotionCtrl, "ApplyRecoil", 0);
    }
    if (g_NewItemProp) {
        g_GetAccuracy = il2cpp_class_get_method_from_name(g_NewItemProp, "get_Accuracy", 0);
        g_GetStability = il2cpp_class_get_method_from_name(g_NewItemProp, "get_Stability", 0);
    }

    // GMStateManager - for infinite ammo
    g_GMStateManager = il2cpp_class_from_name(imgCSharp, "", "GMStateManager");
    if (g_GMStateManager) {
        g_SetNoCostBullet = il2cpp_class_get_method_from_name(g_GMStateManager, "SetNoCostBullet", 1);
    }

    // FOV functions (direct RVA)
    g_GetFOV = (GetFOV_t)((BYTE*)g_GameAssembly + RVA_CAMERA_GET_FOV);
    g_SetFOV = (SetFOV_t)((BYTE*)g_GameAssembly + RVA_CAMERA_SET_FOV);

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

    // Check if already full
    auto curBulletObj = il2cpp_runtime_invoke(g_GetCurBullet, itemProp, nullptr, nullptr);
    if (curBulletObj) {
        int curBullet = *(int*)il2cpp_object_unbox(curBulletObj);
        if (curBullet >= maxBullet) return; // Already full, skip
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
    // Teleport all dropped items to player position
    if (!g_PlayerDictField || !g_MainCtrlField || !g_GetPositionInjected || !g_SetPositionInjected)
        return;
    
    // Get MainCtrl (local player ID)
    int mainCtrl = 0;
    il2cpp_field_static_get_value(g_MainCtrlField, &mainCtrl);
    if (mainCtrl == 0) return;
    
    // Get PlayerDict
    Il2CppObject* playerDict = nullptr;
    il2cpp_field_static_get_value(g_PlayerDictField, &playerDict);
    if (!playerDict) return;
    
    // Dictionary structure: entries at 0x18, count at 0x20
    auto entries = *(Il2CppObject**)((char*)playerDict + 0x18);
    int dictCount = *(int*)((char*)playerDict + 0x20);
    if (!entries || dictCount <= 0) return;
    
    // First find player transform and position
    Il2CppObject* playerTrans = nullptr;
    for (int i = 0; i < dictCount + 50; i++) {
        // Entry: hashCode(4), next(4), key(4), padding(4), value(8) = 24 bytes
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
    
    // Get player position
    Vector3 playerPos;
    g_GetPositionInjected(playerTrans, &playerPos);
    
    // Target position (slightly above player)
    Vector3 targetPos = { playerPos.x, playerPos.y + 0.5f, playerPos.z };
    
    // Iterate and teleport all drops
    for (int i = 0; i < dictCount + 50; i++) {
        char* entryBase = (char*)entries + 0x20 + i * 24;
        int hashCode = *(int*)entryBase;
        if (hashCode < 0) continue;
        
        auto playerObj = *(Il2CppObject**)(entryBase + 0x10);
        if (!playerObj) continue;
        
        // Check if has DropOPCom (is a drop item)
        auto dropCom = *(Il2CppObject**)((char*)playerObj + OFFSET_DROPOPCOM);
        if (!dropCom) continue;
        
        // Get gameTrans
        auto gameTrans = *(Il2CppObject**)((char*)playerObj + OFFSET_GAMETRANS);
        if (!gameTrans) continue;
        
        // Teleport to player
        g_SetPositionInjected(gameTrans, &targetPos);
    }
}

// Update cached target - called from main loop only
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
        
        // Limit to prevent GC issues with too many monsters
        // Reduced from 30 to 15 for better performance
        if (count > 15) count = 15;
        
        // Calculate aim direction
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
            
            bool findNearest = true;
            void* weakArgs[] = { &findNearest };
            auto weakTrans = il2cpp_runtime_invoke(g_GetWeakTrans, bodyPartCom, weakArgs, nullptr);
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
        
        // Thread-safe update using SRWLOCK
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

// Get cached target - thread-safe read for hooks using shared lock
bool GetCachedTarget(Vector3* outPos) {
    // Early exit if shutting down
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

// Hooked EnableCtrl - uses cached target, no IL2CPP calls
void HookedEnableCtrl(void* thisPtr, void* skill, Vector3* start, Vector3* showstart, Vector3* end,
    int pierce, float distance, float speed, int targettype, void* effect, float liveTime,
    void* trailEffect, float effectLiveTime, float radius, float flyoverdis, int passid, int extCheck)
{
    // Safety check - don't process if shutting down or hooks not installed
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

    // Fast bullet
    float finalSpeed = g_FastBullet ? speed * g_BulletSpeedMultiplier : speed;

    g_OriginalEnableCtrl(thisPtr, skill, start, showstart, end, pierce, distance, finalSpeed,
        targettype, effect, liveTime, trailEffect, effectLiveTime, radius, flyoverdis, passid, extCheck);
}

// Hooked Enable - uses cached target, no IL2CPP calls
void HookedEnable(void* thisPtr, void* skill, Vector3* start, Vector3* checkstart, Vector3* end,
    int pierce, float distance, float speed, int targettype, void* effect, float liveTime,
    void* traileffect, float effectlivetime, float radius, float flyoverdis)
{
    // Safety check - don't process if shutting down or hooks not installed
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

    // Fast bullet
    float finalSpeed = g_FastBullet ? speed * g_BulletSpeedMultiplier : speed;

    g_OriginalEnable(thisPtr, skill, start, checkstart, end, pierce, distance, finalSpeed,
        targettype, effect, liveTime, traileffect, effectlivetime, radius, flyoverdis);
}

// Hooked ThrowByPowerCartoon.EnableCtrl - for skill aimbot (grenade, etc.)
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

    // Skill aimbot - aim at closest monster
    if (g_SkillAimEnabled && dir && targettype == 1) {
        Vector3 targetPos;
        if (GetCachedTarget(&targetPos)) {
            // Calculate direction to monster
            Vector3 toMonster = {
                targetPos.x - start->x,
                targetPos.y - start->y,
                targetPos.z - start->z
            };
            float dist = sqrtf(toMonster.x*toMonster.x + toMonster.y*toMonster.y + toMonster.z*toMonster.z);
            if (dist > 0.5f) {
                // Normalize direction
                finalDir.x = toMonster.x / dist;
                finalDir.y = toMonster.y / dist;
                finalDir.z = toMonster.z / dist;
                
                // Fast speed for straight trajectory
                finalSpeed = speed * g_SkillSpeedMultiplier;
                
                // Remove upward force for straight line
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

// NoRecoil hooks - simply do nothing when called
void HookedCameraCtrlRecoil(void* thisPtr) {
    if (!g_NoRecoil && g_OriginalCameraCtrlRecoil) {
        g_OriginalCameraCtrlRecoil(thisPtr);
    }
    // When g_NoRecoil is true, do nothing
}

void HookedSightLogicRecoil(void* thisPtr) {
    if (!g_NoRecoil && g_OriginalSightLogicRecoil) {
        g_OriginalSightLogicRecoil(thisPtr);
    }
}

void HookedSightLogicBulletRecoil(void* thisPtr) {
    if (!g_NoRecoil && g_OriginalSightLogicBulletRecoil) {
        g_OriginalSightLogicBulletRecoil(thisPtr);
    }
}

void HookedWeaponMotionCtrlApplyRecoil(void* thisPtr) {
    if (!g_NoRecoil && g_OriginalWeaponMotionCtrlApplyRecoil) {
        g_OriginalWeaponMotionCtrlApplyRecoil(thisPtr);
    }
}

// NoSpread hooks - return 0 for no spread
float HookedGetCurDis(void* thisPtr) {
    if (g_NoSpread) {
        return 0.0f;
    }
    return g_OriginalGetCurDis ? g_OriginalGetCurDis(thisPtr) : 0.0f;
}

float HookedGetCurBulletTraceRadius(void* thisPtr) {
    if (g_NoSpread) {
        return 0.0f;
    }
    return g_OriginalGetCurBulletTraceRadius ? g_OriginalGetCurBulletTraceRadius(thisPtr) : 0.0f;
}

// Infinite Ammo hook - always return a huge number for current bullets
int HookedGetCurBullet(void* thisPtr) {
    if (g_InfiniteAmmo) {
        return 9999;
    }
    return g_OriginalGetCurBullet ? g_OriginalGetCurBullet(thisPtr) : 100;
}

// FOV hook - capture camera and apply custom FOV
GetFOV_t g_OriginalGetFOV = nullptr;
float HookedGetFOV(void* camera) {
    // Cache camera pointer
    g_CachedCamera = camera;
    
    float originalFOV = g_OriginalGetFOV ? g_OriginalGetFOV(camera) : 60.0f;
    
    // Store original FOV if not set
    if (g_OriginalFOV == 0.0f) {
        g_OriginalFOV = originalFOV;
    }
    
    // Return custom FOV if enabled
    if (g_FOVEnabled) {
        return g_CustomFOV;
    }
    
    return originalFOV;
}

// Store hook addresses for proper cleanup
void* g_EnableCtrlAddr = nullptr;
void* g_EnableAddr = nullptr;
void* g_ThrowEnableCtrlAddr = nullptr;
void* g_CameraCtrlRecoilAddr = nullptr;
void* g_SightLogicRecoilAddr = nullptr;
void* g_SightLogicBulletRecoilAddr = nullptr;
void* g_WeaponMotionCtrlApplyRecoilAddr = nullptr;
void* g_GetAccuracyAddr = nullptr;
void* g_GetStabilityAddr = nullptr;
void* g_GetCurBulletAddr = nullptr;
void* g_GetCurDisAddr = nullptr;
void* g_GetCurBulletTraceRadiusAddr = nullptr;
void* g_GetFOVAddr = nullptr;

bool InstallHooks() {
    if (MH_Initialize() != MH_OK) {
        printf("[GFR Mod] Failed to initialize MinHook\n");
        return false;
    }

    g_EnableCtrlAddr = (BYTE*)g_GameAssembly + RVA_RAYCAST_ENABLE_CTRL;
    g_EnableAddr = (BYTE*)g_GameAssembly + RVA_RAYCAST_ENABLE;

    if (MH_CreateHook(g_EnableCtrlAddr, (void*)HookedEnableCtrl, (void**)&g_OriginalEnableCtrl) != MH_OK) {
        printf("[GFR Mod] Failed to create EnableCtrl hook\n");
        MH_Uninitialize();
        return false;
    }

    if (MH_CreateHook(g_EnableAddr, (void*)HookedEnable, (void**)&g_OriginalEnable) != MH_OK) {
        printf("[GFR Mod] Failed to create Enable hook\n");
        MH_RemoveHook(g_EnableCtrlAddr);
        MH_Uninitialize();
        return false;
    }

    // ThrowByPowerCartoon hook (for skill aimbot - grenade, etc.)
    g_ThrowEnableCtrlAddr = (BYTE*)g_GameAssembly + RVA_THROW_ENABLE_CTRL;
    MH_CreateHook(g_ThrowEnableCtrlAddr, (void*)HookedThrowEnableCtrl, (void**)&g_OriginalThrowEnableCtrl);

    // NoRecoil hooks - use direct RVA instead of il2cpp_method_get_pointer
    g_CameraCtrlRecoilAddr = (BYTE*)g_GameAssembly + RVA_CAMERACTRL_RECOIL;
    MH_CreateHook(g_CameraCtrlRecoilAddr, (void*)HookedCameraCtrlRecoil, (void**)&g_OriginalCameraCtrlRecoil);

    g_SightLogicRecoilAddr = (BYTE*)g_GameAssembly + RVA_SIGHTLOGIC_RECOIL;
    MH_CreateHook(g_SightLogicRecoilAddr, (void*)HookedSightLogicRecoil, (void**)&g_OriginalSightLogicRecoil);

    g_SightLogicBulletRecoilAddr = (BYTE*)g_GameAssembly + RVA_SIGHTLOGIC_BULLETRECOIL;
    MH_CreateHook(g_SightLogicBulletRecoilAddr, (void*)HookedSightLogicBulletRecoil, (void**)&g_OriginalSightLogicBulletRecoil);

    g_WeaponMotionCtrlApplyRecoilAddr = (BYTE*)g_GameAssembly + RVA_WEAPONMOTIONCTRL_APPLYRECOIL;
    MH_CreateHook(g_WeaponMotionCtrlApplyRecoilAddr, (void*)HookedWeaponMotionCtrlApplyRecoil, (void**)&g_OriginalWeaponMotionCtrlApplyRecoil);

    // NoSpread hooks - use direct RVA
    g_GetCurDisAddr = (BYTE*)g_GameAssembly + RVA_SIGHTLOGIC_GETCURDIS;
    MH_CreateHook(g_GetCurDisAddr, (void*)HookedGetCurDis, (void**)&g_OriginalGetCurDis);

    g_GetCurBulletTraceRadiusAddr = (BYTE*)g_GameAssembly + RVA_SIGHTLOGIC_GETCURBULLETTRACERADIUS;
    MH_CreateHook(g_GetCurBulletTraceRadiusAddr, (void*)HookedGetCurBulletTraceRadius, (void**)&g_OriginalGetCurBulletTraceRadius);

    // Infinite Ammo hook - get_CurBullet
    if (il2cpp_method_get_pointer && g_GetCurBullet) {
        g_GetCurBulletAddr = il2cpp_method_get_pointer(g_GetCurBullet);
        if (g_GetCurBulletAddr) {
            MH_CreateHook(g_GetCurBulletAddr, (void*)HookedGetCurBullet, (void**)&g_OriginalGetCurBullet);
        }
    }

    // FOV hook - get_fieldOfView
    g_GetFOVAddr = (BYTE*)g_GameAssembly + RVA_CAMERA_GET_FOV;
    MH_CreateHook(g_GetFOVAddr, (void*)HookedGetFOV, (void**)&g_OriginalGetFOV);

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        printf("[GFR Mod] Failed to enable hooks\n");
        MH_RemoveHook(g_EnableCtrlAddr);
        MH_RemoveHook(g_EnableAddr);
        MH_Uninitialize();
        return false;
    }

    g_HooksInstalled.store(true, std::memory_order_release);
    return true;
}

void RemoveHooks() {
    // Signal shutdown first - prevents hooks from doing work
    g_ShuttingDown.store(true, std::memory_order_release);

    // Small delay to let any in-flight hook calls complete
    Sleep(50);

    // Mark hooks as uninstalled
    g_HooksInstalled.store(false, std::memory_order_release);

    // Disable all hooks
    MH_DisableHook(MH_ALL_HOOKS);

    // Remove all hooks
    if (g_EnableAddr) MH_RemoveHook(g_EnableAddr);
    if (g_EnableCtrlAddr) MH_RemoveHook(g_EnableCtrlAddr);
    if (g_ThrowEnableCtrlAddr) MH_RemoveHook(g_ThrowEnableCtrlAddr);
    if (g_CameraCtrlRecoilAddr) MH_RemoveHook(g_CameraCtrlRecoilAddr);
    if (g_SightLogicRecoilAddr) MH_RemoveHook(g_SightLogicRecoilAddr);
    if (g_SightLogicBulletRecoilAddr) MH_RemoveHook(g_SightLogicBulletRecoilAddr);
    if (g_WeaponMotionCtrlApplyRecoilAddr) MH_RemoveHook(g_WeaponMotionCtrlApplyRecoilAddr);
    if (g_GetAccuracyAddr) MH_RemoveHook(g_GetAccuracyAddr);
    if (g_GetStabilityAddr) MH_RemoveHook(g_GetStabilityAddr);
    if (g_GetCurBulletAddr) MH_RemoveHook(g_GetCurBulletAddr);
    if (g_GetCurDisAddr) MH_RemoveHook(g_GetCurDisAddr);
    if (g_GetCurBulletTraceRadiusAddr) MH_RemoveHook(g_GetCurBulletTraceRadiusAddr);
    if (g_GetFOVAddr) MH_RemoveHook(g_GetFOVAddr);

    MH_Uninitialize();

    // Clear cached target to prevent stale data
    AcquireSRWLockExclusive(&g_TargetLock);
    g_CachedTarget.valid = false;
    g_CachedTarget.x = 0;
    g_CachedTarget.y = 0;
    g_CachedTarget.z = 0;
    ReleaseSRWLockExclusive(&g_TargetLock);

    printf("[GFR Mod] Hooks removed safely\n");
}

void MainThread(HMODULE hModule) {
    AllocConsole();
    FILE* f; freopen_s(&f, "CONOUT$", "w", stdout);

    Sleep(2000);

    if (!InitIL2CPP()) {
        printf("[GFR Mod] Failed to init IL2CPP\n");
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }

    // CRITICAL: Attach this thread to IL2CPP GC
    // Without this, GC will crash when collecting from our thread
    auto domain = il2cpp_domain_get();
    if (domain) {
        il2cpp_thread_attach(domain);
    }
    
    if (!InstallHooks()) {
        printf("[GFR Mod] Failed to install hooks\n");
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }
    
    printf("[GFR Mod] Ready!\n");
    printf("  F1 = Silent Aim (%s)\n", g_SilentAimEnabled ? "ON" : "OFF");
    printf("  F2 = Infinite Ammo (%s)\n", g_InfiniteAmmo ? "ON" : "OFF");
    printf("  F3 = Speed Boost (%s)\n", g_SpeedBoost ? "ON" : "OFF");
    printf("  F4 = No Recoil (%s)\n", g_NoRecoil ? "ON" : "OFF");
    printf("  F5 = No Spread (%s)\n", g_NoSpread ? "ON" : "OFF");
    printf("  F6 = Fast Bullet (%s)\n", g_FastBullet ? "ON" : "OFF");
    printf("  F7 = Skill Aim (%s)\n", g_SkillAimEnabled ? "ON" : "OFF");
    printf("  F8 = FOV Hack (%s, %.0f)\n", g_FOVEnabled ? "ON" : "OFF", g_CustomFOV);
    printf("  Mouse Wheel = Auto Pickup\n");
    printf("  END = Exit\n");

    // Initialize default ON features
    if (g_InfiniteAmmo && g_SetNoCostBullet) {
        bool enable = true;
        void* args[] = { &enable };
        il2cpp_runtime_invoke(g_SetNoCostBullet, nullptr, args, nullptr);
    }

    while (g_Running) {
        if (GetAsyncKeyState(VK_END) & 1) break;

        if (GetAsyncKeyState(VK_F1) & 1) {
            g_SilentAimEnabled = !g_SilentAimEnabled;
            printf("[GFR Mod] Silent Aim: %s\n", g_SilentAimEnabled ? "ON" : "OFF");
        }

        if (GetAsyncKeyState(VK_F2) & 1) {
            g_InfiniteAmmo = !g_InfiniteAmmo;
            if (g_SetNoCostBullet) {
                bool enable = g_InfiniteAmmo;
                void* args[] = { &enable };
                il2cpp_runtime_invoke(g_SetNoCostBullet, nullptr, args, nullptr);
            }
            printf("[GFR Mod] Infinite Ammo: %s\n", g_InfiniteAmmo ? "ON" : "OFF");
        }

        if (GetAsyncKeyState(VK_F3) & 1) {
            g_SpeedBoost = !g_SpeedBoost;
            printf("[GFR Mod] Speed Boost: %s\n", g_SpeedBoost ? "ON" : "OFF");

            auto localPlayer = GetLocalPlayer();
            if (localPlayer) {
                if (g_SpeedBoost) {
                    g_OriginalSpeed = GetPlayerSpeed(localPlayer);
                    g_OriginalJumpHeight = GetJumpHeight(localPlayer);
                    SetPlayerSpeed(localPlayer, g_BoostedSpeed);
                    SetJumpHeight(localPlayer, g_BoostedJumpHeight);
                } else {
                    SetPlayerSpeed(localPlayer, g_OriginalSpeed > 0 ? g_OriginalSpeed : 500);
                    SetJumpHeight(localPlayer, g_OriginalJumpHeight > 0 ? g_OriginalJumpHeight : 1.0f);
                }
            }
        }

        if (GetAsyncKeyState(VK_F4) & 1) {
            g_NoRecoil = !g_NoRecoil;
            printf("[GFR Mod] No Recoil: %s\n", g_NoRecoil ? "ON" : "OFF");
        }

        if (GetAsyncKeyState(VK_F5) & 1) {
            g_NoSpread = !g_NoSpread;
            printf("[GFR Mod] No Spread: %s\n", g_NoSpread ? "ON" : "OFF");
        }

        if (GetAsyncKeyState(VK_F6) & 1) {
            g_FastBullet = !g_FastBullet;
            printf("[GFR Mod] Fast Bullet: %s\n", g_FastBullet ? "ON" : "OFF");
        }

        if (GetAsyncKeyState(VK_F7) & 1) {
            g_SkillAimEnabled = !g_SkillAimEnabled;
            printf("[GFR Mod] Skill Aim: %s\n", g_SkillAimEnabled ? "ON" : "OFF");
        }

        if (GetAsyncKeyState(VK_F8) & 1) {
            g_FOVEnabled = !g_FOVEnabled;
            printf("[GFR Mod] FOV Hack: %s (%.0f)\n", g_FOVEnabled ? "ON" : "OFF", g_CustomFOV);
        }

        if (GetAsyncKeyState(VK_MBUTTON) & 1) {
            if (g_OneGPU || g_StartGPU) {
                AutoPickup();
                printf("[GFR Mod] Auto Pickup - picked up ALL items on map!\n");
            }
        }

        __try {
            // Cache local player once per frame
            auto localPlayer = GetLocalPlayer();
            DWORD now = GetTickCount();

            // Infinite Ammo is handled by hook (HookedGetCurBullet)

            if (g_SpeedBoost && localPlayer && (now - g_LastSpeedCheck > 500)) {
                g_LastSpeedCheck = now;
                int curSpeed = GetPlayerSpeed(localPlayer);
                if (curSpeed != g_BoostedSpeed) {
                    SetPlayerSpeed(localPlayer, g_BoostedSpeed);
                }
            }
            
            // Update cached target for silent aim (every frame)
            if (g_SilentAimEnabled && g_GetMainCameraCom && g_GetTransform) {
                auto camera = il2cpp_runtime_invoke(g_GetMainCameraCom, nullptr, nullptr, nullptr);
                if (camera) {
                    auto camTrans = il2cpp_runtime_invoke(g_GetTransform, camera, nullptr, nullptr);
                    if (camTrans) {
                        auto posObj = il2cpp_runtime_invoke(g_GetPosition, camTrans, nullptr, nullptr);
                        auto fwdObj = il2cpp_runtime_invoke(g_GetForward, camTrans, nullptr, nullptr);
                        if (posObj && fwdObj) {
                            Vector3 camPos = *(Vector3*)il2cpp_object_unbox(posObj);
                            Vector3 camFwd = *(Vector3*)il2cpp_object_unbox(fwdObj);
                            Vector3 aimEnd = {
                                camPos.x + camFwd.x * 1000.0f,
                                camPos.y + camFwd.y * 1000.0f,
                                camPos.z + camFwd.z * 1000.0f
                            };
                            UpdateCachedTarget(&camPos, &aimEnd);
                        }
                    }
                }
            }
            
        } __except(EXCEPTION_EXECUTE_HANDLER) {
        }

        Sleep(8);
    }
    
    RemoveHooks();
    
    if (g_SpeedBoost) {
        __try {
            auto localPlayer = GetLocalPlayer();
            if (localPlayer && g_OriginalSpeed > 0) {
                SetPlayerSpeed(localPlayer, g_OriginalSpeed);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    
    printf("[GFR Mod] Exiting...\n");
    Sleep(500);
    if (f) fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        std::thread(MainThread, hModule).detach();
    }
    return TRUE;
}
