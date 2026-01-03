#include "hooks.h"
#include "../il2cpp/il2cpp.h"
#include "../game/game.h"
#include "../features/settings.h"
#include "../features/aimbot.h"
#include "../features/norecoil.h"
#include "../features/fov.h"
#include "../features/weakness_hack.h"
#include "../render/dx11.h"
#include <MinHook.h>
#include <cstdio>

bool InstallHooks() {
    if (MH_Initialize() != MH_OK) {
        printf("[GFR Mod] Failed to initialize MinHook\n");
        return false;
    }

    // Aimbot hooks
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

    // Skill aimbot hook
    g_ThrowEnableCtrlAddr = (BYTE*)g_GameAssembly + RVA_THROW_ENABLE_CTRL;
    MH_CreateHook(g_ThrowEnableCtrlAddr, (void*)HookedThrowEnableCtrl, (void**)&g_OriginalThrowEnableCtrl);

    // NoRecoil hooks
    g_CameraCtrlRecoilAddr = (BYTE*)g_GameAssembly + RVA_CAMERACTRL_RECOIL;
    MH_CreateHook(g_CameraCtrlRecoilAddr, (void*)HookedCameraCtrlRecoil, (void**)&g_OriginalCameraCtrlRecoil);

    g_SightLogicRecoilAddr = (BYTE*)g_GameAssembly + RVA_SIGHTLOGIC_RECOIL;
    MH_CreateHook(g_SightLogicRecoilAddr, (void*)HookedSightLogicRecoil, (void**)&g_OriginalSightLogicRecoil);

    g_SightLogicBulletRecoilAddr = (BYTE*)g_GameAssembly + RVA_SIGHTLOGIC_BULLETRECOIL;
    MH_CreateHook(g_SightLogicBulletRecoilAddr, (void*)HookedSightLogicBulletRecoil, (void**)&g_OriginalSightLogicBulletRecoil);

    g_WeaponMotionCtrlApplyRecoilAddr = (BYTE*)g_GameAssembly + RVA_WEAPONMOTIONCTRL_APPLYRECOIL;
    MH_CreateHook(g_WeaponMotionCtrlApplyRecoilAddr, (void*)HookedWeaponMotionCtrlApplyRecoil, (void**)&g_OriginalWeaponMotionCtrlApplyRecoil);

    // NoSpread hooks
    g_GetCurDisAddr = (BYTE*)g_GameAssembly + RVA_SIGHTLOGIC_GETCURDIS;
    MH_CreateHook(g_GetCurDisAddr, (void*)HookedGetCurDis, (void**)&g_OriginalGetCurDis);

    g_GetCurBulletTraceRadiusAddr = (BYTE*)g_GameAssembly + RVA_SIGHTLOGIC_GETCURBULLETTRACERADIUS;
    MH_CreateHook(g_GetCurBulletTraceRadiusAddr, (void*)HookedGetCurBulletTraceRadius, (void**)&g_OriginalGetCurBulletTraceRadius);

    // Infinite Ammo hook
    if (il2cpp_method_get_pointer && g_GetCurBullet) {
        g_GetCurBulletAddr = il2cpp_method_get_pointer(g_GetCurBullet);
        if (g_GetCurBulletAddr) {
            MH_CreateHook(g_GetCurBulletAddr, (void*)HookedGetCurBullet, (void**)&g_OriginalGetCurBullet);
        }
    }

    // FOV hook
    g_GetFOVAddr = (BYTE*)g_GameAssembly + RVA_CAMERA_GET_FOV;
    MH_CreateHook(g_GetFOVAddr, (void*)HookedGetFOV, (void**)&g_OriginalGetFOV);

    // Weakness hit hack hooks
    InstallWeaknessHooks();

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
    g_ShuttingDown.store(true, std::memory_order_release);
    Sleep(50);
    g_HooksInstalled.store(false, std::memory_order_release);

    CleanupDX11();

    MH_DisableHook(MH_ALL_HOOKS);

    if (g_EnableAddr) MH_RemoveHook(g_EnableAddr);
    if (g_EnableCtrlAddr) MH_RemoveHook(g_EnableCtrlAddr);
    if (g_ThrowEnableCtrlAddr) MH_RemoveHook(g_ThrowEnableCtrlAddr);
    if (g_CameraCtrlRecoilAddr) MH_RemoveHook(g_CameraCtrlRecoilAddr);
    if (g_SightLogicRecoilAddr) MH_RemoveHook(g_SightLogicRecoilAddr);
    if (g_SightLogicBulletRecoilAddr) MH_RemoveHook(g_SightLogicBulletRecoilAddr);
    if (g_WeaponMotionCtrlApplyRecoilAddr) MH_RemoveHook(g_WeaponMotionCtrlApplyRecoilAddr);
    if (g_GetCurDisAddr) MH_RemoveHook(g_GetCurDisAddr);
    if (g_GetCurBulletTraceRadiusAddr) MH_RemoveHook(g_GetCurBulletTraceRadiusAddr);
    if (g_GetCurBulletAddr) MH_RemoveHook(g_GetCurBulletAddr);
    if (g_GetFOVAddr) MH_RemoveHook(g_GetFOVAddr);
    if (g_PresentAddr) MH_RemoveHook(g_PresentAddr);
    
    // Weakness hooks
    RemoveWeaknessHooks();

    MH_Uninitialize();

    AcquireSRWLockExclusive(&g_TargetLock);
    g_CachedTarget.valid = false;
    g_CachedTarget.x = 0;
    g_CachedTarget.y = 0;
    g_CachedTarget.z = 0;
    ReleaseSRWLockExclusive(&g_TargetLock);

    printf("[GFR Mod] Hooks removed safely\n");
}
