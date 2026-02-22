#pragma once
#include <atomic>
#include <Windows.h>

// Global settings
extern bool g_Running;
extern bool g_SilentAimEnabled;
extern bool g_InfiniteAmmo;
extern bool g_SpeedBoost;
extern bool g_NoRecoil;
extern bool g_NoSpread;
extern bool g_FastBullet;
extern bool g_ESPEnabled;
extern bool g_MenuVisible;
extern bool g_WeaknessHack;
extern bool g_NoCooldown;
extern bool g_FullMapReveal;
extern bool g_InfiniteGrenades;
extern bool g_MaxDefense;
extern float g_BulletSpeedMultiplier;
extern int g_OriginalSpeed;
extern int g_BoostedSpeed;
extern float g_OriginalJumpHeight;
extern float g_BoostedJumpHeight;

// Thread safety
extern std::atomic<bool> g_ShuttingDown;
extern std::atomic<bool> g_HooksInstalled;
extern SRWLOCK g_TargetLock;
extern SRWLOCK g_ESPLock;
extern SRWLOCK g_MatrixLock;
