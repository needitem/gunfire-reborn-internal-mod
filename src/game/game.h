#pragma once
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

// RVAs
constexpr DWORD RVA_RAYCAST_ENABLE_CTRL = 0x01364c90;
constexpr DWORD RVA_RAYCAST_ENABLE = 0x01365450;
constexpr DWORD RVA_GET_POSITION_INJECTED = 0x03783270;
constexpr DWORD RVA_SET_POSITION_INJECTED = 0x03783eb0;
constexpr DWORD RVA_PHYSICS_LINECAST = 0x036730d0;
constexpr DWORD RVA_THROW_ENABLE_CTRL = 0x014f0fb0;
constexpr DWORD RVA_THROW_ENABLE = 0x014f1de0;
constexpr DWORD RVA_CAMERACTRL_RECOIL = 0x00d8fc90;
constexpr DWORD RVA_SIGHTLOGIC_RECOIL = 0x00a7ebf0;
constexpr DWORD RVA_SIGHTLOGIC_BULLETRECOIL = 0x00a7b7a0;
constexpr DWORD RVA_WEAPONMOTIONCTRL_APPLYRECOIL = 0x00fff630;
constexpr DWORD RVA_SIGHTLOGIC_GETCURDIS = 0x00a7c3b0;
constexpr DWORD RVA_SIGHTLOGIC_GETCURBULLETTRACERADIUS = 0x00a7c380;
constexpr DWORD RVA_CAMERA_GET_FOV = 0x025d1480;
constexpr DWORD RVA_CAMERA_SET_FOV = 0x025d22c0;
constexpr DWORD RVA_GET_WARCASH = 0x00b85720;
constexpr DWORD RVA_SET_WARCASH = 0x00b87d10;
constexpr DWORD RVA_GET_PLAYERPROP = 0x00c17b40;
constexpr DWORD RVA_CAMERA_WORLDTOCAMERAMATRIX = 0x025d1d70;
constexpr DWORD RVA_CAMERA_PROJECTIONMATRIX = 0x025d1860;
constexpr DWORD RVA_GET_MAINCAMERACOM_DIRECT = 0x00d92a00;

// Weakness hit hack RVAs
constexpr DWORD RVA_CARTOONDATA_SETSKILLLRAY = 0x01925f80;
constexpr DWORD RVA_CARTOONDATA_PACKETSKILLRAY = 0x01924bd0;
constexpr DWORD RVA_SCLIENTHITINFO_CTOR = 0x010dd160;

// FightType values
constexpr int FIGHTTYPE_OBSTACLE_NORMAL = 268435713;
constexpr int FIGHTTYPE_NPC_TRANSFER = 33554434;
constexpr int FIGHTTYPE_NPC_TREASUREBOX = 33554448;
constexpr int FIGHTTYPE_NPC_EVENT = 33554441;
constexpr int SID_SECRET_WALL = 1015;
constexpr int SID_SECRET_WALL_2 = 1016;
constexpr int SID_SECRET_WALL_3 = 1026;
constexpr int SID_SECRET_WALL_4 = 1044;  // 3라운드 비밀방
constexpr int SID_SECRET_PORTAL = 1017;
constexpr int SID_SECRET_PORTAL_2 = 1019;

// Function pointer types
typedef void (*GetPositionInjected_t)(void* transform, Vector3* ret);
typedef void (*SetPositionInjected_t)(void* transform, Vector3* value);
typedef void (*GetMatrix_t)(void* camera, Matrix4x4* ret);
typedef void* (*GetMainCameraComDirect_t)();
typedef int (*GetWarCash_t)(void* playerProp);
typedef void (*SetWarCash_t)(void* playerProp, int value);
typedef void* (*GetPlayerProp_t)(int pid);
typedef float (*GetFOV_t)(void* camera);
typedef void (*SetFOV_t)(void* camera, float fov);

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
extern Il2CppMethod* g_GetSpecialWeakTrans;
extern Il2CppMethod* g_GetWeakTransByTag;

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
extern GetFOV_t g_GetFOV;
extern SetFOV_t g_SetFOV;

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
