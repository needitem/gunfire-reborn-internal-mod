#pragma once
#include <cstdint>
#include "../il2cpp/il2cpp.h"

// Vector3 structure
struct Vector3 {
    float x, y, z;
};

// Matrix4x4 structure
struct Matrix4x4 {
    float m[16];
};

// Offsets
constexpr size_t OFFSET_PLAYERCOM = 0x128;
constexpr size_t OFFSET_CURWEAPONASSLOT = 0x68;
constexpr size_t OFFSET_FSTASOBJ = 0x50;
constexpr size_t OFFSET_RELOADCOM = 0x10;
constexpr size_t OFFSET_ITEMPROP = 0x18;
constexpr size_t OFFSET_PLAYERPROP = 0x48;
constexpr size_t OFFSET_BODYPARTCOM = 0x160;
constexpr size_t OFFSET_GAMETRANS = 0x88;
constexpr size_t OFFSET_DROPOPCOM = 0xd0;
constexpr size_t OFFSET_SID = 0x24;
constexpr size_t OFFSET_FIGHTTYPE = 0x7c;

// All methods are resolved at runtime via IL2CPP API (no hardcoded RVAs)

// FightType values
constexpr int FIGHTTYPE_OBSTACLE_NORMAL = 268435713;
constexpr int FIGHTTYPE_NPC_TRANSFER = 33554434;
constexpr int FIGHTTYPE_NPC_TREASUREBOX = 33554448;
constexpr int FIGHTTYPE_NPC_EVENT = 33554441;
constexpr int SID_SECRET_WALL = 1015;
constexpr int SID_SECRET_WALL_2 = 1016;
constexpr int SID_SECRET_WALL_3 = 1026;
constexpr int SID_SECRET_WALL_4 = 1044;
constexpr int SID_SECRET_WALL_5 = 1045;
constexpr int SID_SECRET_PORTAL = 1017;
constexpr int SID_SECRET_PORTAL_2 = 1019;
constexpr int SID_SECRET_PORTAL_3 = 1057;

// Function pointer types
typedef void (*GetPositionInjected_t)(void* transform, Vector3* ret);
typedef void (*SetPositionInjected_t)(void* transform, Vector3* value);
typedef void (*GetMatrix_t)(void* camera, Matrix4x4* ret);
typedef void* (*GetMainCameraComDirect_t)();
typedef int (*GetWarCash_t)(void* playerProp);
typedef void (*SetWarCash_t)(void* playerProp, int value);
typedef void* (*GetPlayerProp_t)(int pid);

// Cached methods
extern Il2CppMethod* g_GetMonsters;
extern Il2CppMethod* g_GetPlayer;
extern Il2CppMethod* g_GetMaxBullet;
extern Il2CppMethod* g_GetCurBullet;
extern Il2CppMethod* g_SetCurBullet;
extern Il2CppMethod* g_SetClientCurBullet;
extern Il2CppMethod* g_GetSpeed;
extern Il2CppMethod* g_SetSpeed;
extern Il2CppMethod* g_GetJumpHeight;
extern Il2CppMethod* g_SetJumpHeight;
extern Il2CppMethod* g_OneGPU;
extern Il2CppMethod* g_StartGPU;
extern Il2CppMethod* g_GetWeakTrans;
extern Il2CppMethod* g_GetPosition;
extern Il2CppMethod* g_GetTransform;
extern Il2CppMethod* g_GetForward;
extern Il2CppMethod* g_GetMainCameraCom;
extern Il2CppMethod* g_ListGetCount;
extern Il2CppMethod* g_ListGetItem;
extern Il2CppMethod* g_SetNoCostBullet;

// Cached classes/fields
extern Il2CppClass* g_NewPlayerManager;
extern Il2CppClass* g_NewItemProp;
extern Il2CppClass* g_PlayerProp;
extern Il2CppClass* g_DropManager;
extern Il2CppClass* g_OCBodyPart;
extern Il2CppClass* g_GMStateManager;
extern void* g_MainCtrlField;
extern void* g_PlayerDictField;

// Direct function pointers
extern GetPositionInjected_t g_GetPositionInjected;
extern SetPositionInjected_t g_SetPositionInjected;
extern GetMatrix_t g_GetWorldToCameraMatrix;
extern GetMatrix_t g_GetProjectionMatrix;
extern GetMainCameraComDirect_t g_GetMainCameraComDirect;
extern GetWarCash_t g_GetWarCash;
extern SetWarCash_t g_SetWarCash;
extern GetPlayerProp_t g_GetPlayerPropFunc;

// Game functions
bool InitGame();
Il2CppObject* GetLocalPlayer();
Il2CppObject* GetCurrentWeaponItemProp(Il2CppObject* localPlayer);
void RefillAmmo(Il2CppObject* itemProp);
void SetPlayerSpeed(Il2CppObject* localPlayer, int speed);
int GetPlayerSpeed(Il2CppObject* localPlayer);
void SetJumpHeight(Il2CppObject* localPlayer, float height);
float GetJumpHeight(Il2CppObject* localPlayer);
void AutoPickup();

// Infinite ammo hook
extern void* g_GetNoCostBulletAddr;
typedef bool (*GetNoCostBullet_t)(void* thisPtr, const void* method);
extern GetNoCostBullet_t g_OriginalGetNoCostBullet;
bool HookedGetNoCostBullet(void* thisPtr, const void* method);

// No Cooldown hook
extern void* g_ReturnNocdAddr;
typedef bool (*ReturnNocd_t)(void* thisPtr, const void* method);
extern ReturnNocd_t g_OriginalReturnNocd;
bool HookedReturnNocd(void* thisPtr, const void* method);

// Weakness targeting
void* GetBestWeaknessTrans(void* bodyPartCom);

// Weakness hit hack types
typedef void (*CartoonDataSetSkilllRay_t)(void* thisPtr, void* rayHit);
typedef void (*CartoonDataPacketSkillRay_t)(void* thisPtr, void* lsthit);
typedef void (*SClientHitInfoCtor_t)(void* thisPtr, int vitid, void* luago, void* tran, bool islife,
    Vector3* hitPos, Vector3* hitNormal, void* hitTrans, Vector3* hforward, void* hpart, void* spart);

extern CartoonDataSetSkilllRay_t g_OriginalCartoonDataSetSkilllRay;
extern CartoonDataPacketSkillRay_t g_OriginalCartoonDataPacketSkillRay;
extern SClientHitInfoCtor_t g_OriginalSClientHitInfoCtor;

extern void* g_CartoonDataSetSkilllRayAddr;
extern void* g_CartoonDataPacketSkillRayAddr;
extern void* g_SClientHitInfoCtorAddr;

extern void* g_WeaknessString;
extern void* g_SpecialWeaknessString;

// Reload hook - HeroAttackCtrl.OnReloadBullet
typedef bool (*OnReloadBullet_t)(void* thisPtr, int weaponid, int reason);
extern OnReloadBullet_t g_OriginalOnReloadBullet;
extern void* g_OnReloadBulletAddr;
bool HookedOnReloadBullet(void* thisPtr, int weaponid, int reason);

// Reload callback used by infinite ammo
typedef void (*OnReloadBulletCallBack_t)(void* thisPtr, int weaponid, int flag);
extern OnReloadBulletCallBack_t g_OnReloadBulletCallBack;
extern void* g_OnReloadBulletCallBackAddr;
