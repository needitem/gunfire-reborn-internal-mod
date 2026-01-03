#pragma once
#include <atomic>
#include <Windows.h>

// Global settings
extern bool g_Running;
extern bool g_SilentAimEnabled;
extern bool g_SkillAimEnabled;
extern bool g_InfiniteAmmo;
extern bool g_SpeedBoost;
extern bool g_NoRecoil;
extern bool g_NoSpread;
extern bool g_FastBullet;
extern bool g_FOVEnabled;
extern bool g_BigRadius;
extern bool g_ESPEnabled;
extern bool g_InfiniteGold;
extern bool g_MenuVisible;
extern bool g_WeaknessHack;

extern int g_GoldAmount;
extern float g_CustomFOV;
extern float g_OriginalFOV;
extern float g_BulletSpeedMultiplier;
extern float g_SkillSpeedMultiplier;
extern float g_RadiusValue;
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

// Cached camera
extern void* g_CachedCamera;
