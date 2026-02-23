#include "hooks.h"
#include "../il2cpp/il2cpp.h"
#include "../game/game.h"
#include "../features/settings.h"
#include "../features/aimbot.h"
#include "../features/norecoil.h"
#include "../features/weakness_hack.h"
#include "../render/dx11.h"
#include <MinHook.h>
#include <cstdio>

bool InstallHooks() {
    if (MH_Initialize() != MH_OK) {
        printf("[GFR Mod] Failed to initialize MinHook\n");
        return false;
    }

    // All hook addresses are resolved in InitGame() via IL2CPP API

    // Aimbot hooks
    if (!g_EnableCtrlAddr || !g_EnableAddr) {
        printf("[GFR Mod] Failed to resolve aimbot methods\n");
        MH_Uninitialize();
        return false;
    }

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

    // NoRecoil hooks (Sight_logic.Recoil/BulletRecoil removed in game update)
    if (g_CameraCtrlRecoilAddr) {
        MH_CreateHook(g_CameraCtrlRecoilAddr, (void*)HookedCameraCtrlRecoil, (void**)&g_OriginalCameraCtrlRecoil);
    }
    if (g_WeaponMotionCtrlApplyRecoilAddr) {
        MH_CreateHook(g_WeaponMotionCtrlApplyRecoilAddr, (void*)HookedWeaponMotionCtrlApplyRecoil, (void**)&g_OriginalWeaponMotionCtrlApplyRecoil);
    }

    // NoSpread hooks
    if (g_GetCurDisAddr) {
        MH_CreateHook(g_GetCurDisAddr, (void*)HookedGetCurDis, (void**)&g_OriginalGetCurDis);
    }
    if (g_GetCurBulletTraceRadiusAddr) {
        MH_CreateHook(g_GetCurBulletTraceRadiusAddr, (void*)HookedGetCurBulletTraceRadius, (void**)&g_OriginalGetCurBulletTraceRadius);
    }

    // Infinite ammo hooks
    if (g_GetCurBullet) {
        g_GetCurBulletAddr = il2cpp_method_get_pointer ? il2cpp_method_get_pointer(g_GetCurBullet) : nullptr;
        if (!g_GetCurBulletAddr) {
            // Fallback: MethodInfo.methodPointer at offset 0
            g_GetCurBulletAddr = *(void**)g_GetCurBullet;
        }
        if (g_GetCurBulletAddr) {
            MH_CreateHook(g_GetCurBulletAddr, (void*)HookedGetCurBullet, (void**)&g_OriginalGetCurBullet);
        }
    }
    if (g_GetNoCostBulletAddr) {
        MH_CreateHook(g_GetNoCostBulletAddr, (void*)HookedGetNoCostBullet, (void**)&g_OriginalGetNoCostBullet);
    }
    if (g_SetCurBulletAddr) {
        MH_CreateHook(g_SetCurBulletAddr, (void*)HookedSetCurBullet, (void**)&g_OriginalSetCurBullet);
    }
    if (g_SetClientCurBulletAddr) {
        MH_CreateHook(g_SetClientCurBulletAddr, (void*)HookedSetClientCurBullet, (void**)&g_OriginalSetClientCurBullet);
    }
    if (g_SetCurPFBulletAddr) {
        MH_CreateHook(g_SetCurPFBulletAddr, (void*)HookedSetCurPFBullet, (void**)&g_OriginalSetCurPFBullet);
    }

    // Instant reload behavior is now integrated into infinite ammo.
    if (g_OnReloadBulletAddr) {
        MH_CreateHook(g_OnReloadBulletAddr, (void*)HookedOnReloadBullet, (void**)&g_OriginalOnReloadBullet);
    }

    // No Cooldown hook
    if (g_ReturnNocdAddr) {
        MH_CreateHook(g_ReturnNocdAddr, (void*)HookedReturnNocd, (void**)&g_OriginalReturnNocd);
    }

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
    printf("[GFR Mod] Starting hook removal...\n");

    // Signal shutdown first - this stops all hook functions from doing work
    g_ShuttingDown.store(true, std::memory_order_release);
    g_HooksInstalled.store(false, std::memory_order_release);

    // Wait for any in-progress hook calls to complete
    Sleep(100);

    // Disable all hooks first (makes them call original functions)
    MH_DisableHook(MH_ALL_HOOKS);

    // Wait again for any calls in transition
    Sleep(50);

    // Now cleanup DX11 (ImGui, WndProc, etc.)
    CleanupDX11();

    if (g_EnableAddr) MH_RemoveHook(g_EnableAddr);
    if (g_EnableCtrlAddr) MH_RemoveHook(g_EnableCtrlAddr);
    if (g_CameraCtrlRecoilAddr) MH_RemoveHook(g_CameraCtrlRecoilAddr);
    if (g_WeaponMotionCtrlApplyRecoilAddr) MH_RemoveHook(g_WeaponMotionCtrlApplyRecoilAddr);
    if (g_GetCurDisAddr) MH_RemoveHook(g_GetCurDisAddr);
    if (g_GetCurBulletTraceRadiusAddr) MH_RemoveHook(g_GetCurBulletTraceRadiusAddr);
    if (g_GetCurBulletAddr) MH_RemoveHook(g_GetCurBulletAddr);
    if (g_GetNoCostBulletAddr) MH_RemoveHook(g_GetNoCostBulletAddr);
    if (g_SetCurBulletAddr) MH_RemoveHook(g_SetCurBulletAddr);
    if (g_SetClientCurBulletAddr) MH_RemoveHook(g_SetClientCurBulletAddr);
    if (g_SetCurPFBulletAddr) MH_RemoveHook(g_SetCurPFBulletAddr);
    if (g_OnReloadBulletAddr) MH_RemoveHook(g_OnReloadBulletAddr);
    if (g_ReturnNocdAddr) MH_RemoveHook(g_ReturnNocdAddr);

    // Weakness hooks
    RemoveWeaknessHooks();

    // DX11 Present hook - remove last
    if (g_PresentAddr) MH_RemoveHook(g_PresentAddr);

    // Final wait before uninitialize
    Sleep(50);

    MH_Uninitialize();

    AcquireSRWLockExclusive(&g_TargetLock);
    g_CachedTarget.valid = false;
    g_CachedTarget.x = 0;
    g_CachedTarget.y = 0;
    g_CachedTarget.z = 0;
    ReleaseSRWLockExclusive(&g_TargetLock);

    printf("[GFR Mod] Hooks removed safely\n");
}
