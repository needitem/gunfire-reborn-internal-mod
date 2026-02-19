#include <Windows.h>
#include <cstdio>
#include <thread>
#include "il2cpp/il2cpp.h"
#include "game/game.h"
#include "features/settings.h"
#include "features/aimbot.h"
#include "features/esp.h"
#include "hooks/hooks.h"
#include "render/dx11.h"

static HMODULE g_hModule = nullptr;
static void* g_AttachedThread = nullptr;

void MainThread(HMODULE hModule) {
    g_hModule = hModule;

    AllocConsole();
    FILE* f; freopen_s(&f, "CONOUT$", "w", stdout);

    Sleep(2000);

    if (!InitIL2CPP()) {
        printf("[GFR Mod] Failed to init IL2CPP\n");
        if (f) fclose(f);
        FreeConsole();
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }

    if (!InitGame()) {
        printf("[GFR Mod] Failed to init game\n");
        if (f) fclose(f);
        FreeConsole();
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }

    auto domain = il2cpp_domain_get();
    if (domain && il2cpp_thread_attach) {
        g_AttachedThread = il2cpp_thread_attach(domain);
    }
    
    if (!InstallHooks()) {
        printf("[GFR Mod] Failed to install hooks\n");
        FreeLibraryAndExitThread(hModule, 0);
        return;
    }
    
    if (!InstallDX11Hooks()) {
        printf("[GFR Mod] Warning: DX11 hooks failed, ESP disabled\n");
    }

    printf("[GFR Mod] Ready! Press HOME to open menu\n");

    DWORD lastSpeedCheck = 0;

    while (g_Running) {
        if (GetAsyncKeyState(VK_END) & 1) break;

        if (GetAsyncKeyState(VK_F1) & 1) g_SilentAimEnabled = !g_SilentAimEnabled;
        
        if (GetAsyncKeyState(VK_F2) & 1) {
            g_InfiniteAmmo = !g_InfiniteAmmo;
            // Handled by HookedGetNoCostBullet hook
        }

        if (GetAsyncKeyState(VK_F3) & 1) {
            g_SpeedBoost = !g_SpeedBoost;
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

        if (GetAsyncKeyState(VK_F4) & 1) g_NoRecoil = !g_NoRecoil;
        if (GetAsyncKeyState(VK_F5) & 1) g_NoSpread = !g_NoSpread;
        if (GetAsyncKeyState(VK_F6) & 1) g_FastBullet = !g_FastBullet;
        if (GetAsyncKeyState(VK_F9) & 1) g_BigRadius = !g_BigRadius;
        if (GetAsyncKeyState(VK_F10) & 1) g_WeaknessHack = !g_WeaknessHack;
        if (GetAsyncKeyState(VK_F11) & 1) g_NoCooldown = !g_NoCooldown;

        if (GetAsyncKeyState(VK_MBUTTON) & 1) {
            AutoPickup();
        }

        __try {
            auto localPlayer = GetLocalPlayer();
            DWORD now = GetTickCount();

            // Speed boost maintenance
            if (g_SpeedBoost && localPlayer && (now - lastSpeedCheck > 500)) {
                lastSpeedCheck = now;
                int curSpeed = GetPlayerSpeed(localPlayer);
                if (curSpeed != g_BoostedSpeed) {
                    SetPlayerSpeed(localPlayer, g_BoostedSpeed);
                }
            }

            // Update aimbot target
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
            
            // Update ESP
            if (GetAsyncKeyState(VK_MENU) & 0x8000) {
                UpdateViewMatrix();
                UpdateESPObjects();
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {
        }

        Sleep(8);
    }
    
    printf("[GFR Mod] Shutting down...\n");

    // Signal shutdown first
    g_ShuttingDown.store(true, std::memory_order_release);
    g_Running = false;

    // Restore player speed if modified
    if (g_SpeedBoost) {
        __try {
            auto localPlayer = GetLocalPlayer();
            if (localPlayer && g_OriginalSpeed > 0) {
                SetPlayerSpeed(localPlayer, g_OriginalSpeed);
                SetJumpHeight(localPlayer, g_OriginalJumpHeight > 0 ? g_OriginalJumpHeight : 1.0f);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    // Wait for any pending operations
    Sleep(100);

    // Remove all hooks (this also cleans up DX11)
    RemoveHooks();

    // Detach from IL2CPP thread
    if (g_AttachedThread && il2cpp_thread_detach) {
        __try {
            il2cpp_thread_detach(g_AttachedThread);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
        g_AttachedThread = nullptr;
    }

    // Clear all cached pointers
    g_CachedTarget.valid = false;

    printf("[GFR Mod] Cleanup complete, exiting...\n");
    Sleep(200);

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
