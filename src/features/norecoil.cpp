#include "norecoil.h"
#include "settings.h"

RecoilFunc_t g_OriginalCameraCtrlRecoil = nullptr;
RecoilFunc_t g_OriginalSightLogicRecoil = nullptr;
RecoilFunc_t g_OriginalSightLogicBulletRecoil = nullptr;
RecoilFunc_t g_OriginalWeaponMotionCtrlApplyRecoil = nullptr;
GetFloatFunc_t g_OriginalGetCurDis = nullptr;
GetFloatFunc_t g_OriginalGetCurBulletTraceRadius = nullptr;
GetIntFunc_t g_OriginalGetCurBullet = nullptr;

void* g_CameraCtrlRecoilAddr = nullptr;
void* g_SightLogicRecoilAddr = nullptr;
void* g_SightLogicBulletRecoilAddr = nullptr;
void* g_WeaponMotionCtrlApplyRecoilAddr = nullptr;
void* g_GetCurDisAddr = nullptr;
void* g_GetCurBulletTraceRadiusAddr = nullptr;
void* g_GetCurBulletAddr = nullptr;

void HookedCameraCtrlRecoil(void* thisPtr) {
    if (!g_NoRecoil && g_OriginalCameraCtrlRecoil) {
        g_OriginalCameraCtrlRecoil(thisPtr);
    }
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

int HookedGetCurBullet(void* thisPtr) {
    if (g_InfiniteAmmo) {
        return 9999;
    }
    return g_OriginalGetCurBullet ? g_OriginalGetCurBullet(thisPtr) : 100;
}
