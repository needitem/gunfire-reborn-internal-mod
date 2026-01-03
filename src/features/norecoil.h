#pragma once

// Function types
typedef void (*RecoilFunc_t)(void*);
typedef float (*GetFloatFunc_t)(void*);
typedef int (*GetIntFunc_t)(void*);

// Original function pointers
extern RecoilFunc_t g_OriginalCameraCtrlRecoil;
extern RecoilFunc_t g_OriginalSightLogicRecoil;
extern RecoilFunc_t g_OriginalSightLogicBulletRecoil;
extern RecoilFunc_t g_OriginalWeaponMotionCtrlApplyRecoil;
extern GetFloatFunc_t g_OriginalGetCurDis;
extern GetFloatFunc_t g_OriginalGetCurBulletTraceRadius;
extern GetIntFunc_t g_OriginalGetCurBullet;

// Hook addresses
extern void* g_CameraCtrlRecoilAddr;
extern void* g_SightLogicRecoilAddr;
extern void* g_SightLogicBulletRecoilAddr;
extern void* g_WeaponMotionCtrlApplyRecoilAddr;
extern void* g_GetCurDisAddr;
extern void* g_GetCurBulletTraceRadiusAddr;
extern void* g_GetCurBulletAddr;

// Hooked functions
void HookedCameraCtrlRecoil(void* thisPtr);
void HookedSightLogicRecoil(void* thisPtr);
void HookedSightLogicBulletRecoil(void* thisPtr);
void HookedWeaponMotionCtrlApplyRecoil(void* thisPtr);
float HookedGetCurDis(void* thisPtr);
float HookedGetCurBulletTraceRadius(void* thisPtr);
int HookedGetCurBullet(void* thisPtr);
