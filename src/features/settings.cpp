#include "settings.h"

// Global settings
bool g_Running = true;
bool g_SilentAimEnabled = true;
bool g_SkillAimEnabled = false;
bool g_InfiniteAmmo = true;
bool g_SpeedBoost = true;
bool g_NoRecoil = true;
bool g_NoSpread = true;
bool g_FastBullet = true;
bool g_FOVEnabled = false;
bool g_BigRadius = false;
bool g_ESPEnabled = false;
bool g_InfiniteGold = false;
bool g_MenuVisible = false;
bool g_WeaknessHack = true;

int g_GoldAmount = 99999;
float g_CustomFOV = 120.0f;
float g_OriginalFOV = 0.0f;
float g_BulletSpeedMultiplier = 100.0f;
float g_SkillSpeedMultiplier = 10.0f;
float g_RadiusValue = 999.0f;
int g_OriginalSpeed = 0;
int g_BoostedSpeed = 1000;
float g_OriginalJumpHeight = 0.0f;
float g_BoostedJumpHeight = 1.3f;

// Thread safety
std::atomic<bool> g_ShuttingDown(false);
std::atomic<bool> g_HooksInstalled(false);
SRWLOCK g_TargetLock = SRWLOCK_INIT;
SRWLOCK g_ESPLock = SRWLOCK_INIT;
SRWLOCK g_MatrixLock = SRWLOCK_INIT;

// Cached camera
void* g_CachedCamera = nullptr;
