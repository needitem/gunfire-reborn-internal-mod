#include "settings.h"

// Global settings
bool g_Running = true;
bool g_SilentAimEnabled = true;
bool g_InfiniteAmmo = true;
bool g_SpeedBoost = true;
bool g_NoRecoil = true;
bool g_NoSpread = true;
bool g_FastBullet = true;
bool g_ESPEnabled = false;
bool g_ShowNPCESP = true;
bool g_MenuVisible = false;
bool g_WeaknessHack = true;
bool g_NoCooldown = false;
bool g_FullMapReveal = false;
bool g_InfiniteGrenades = false;
bool g_MaxDefense = false;
float g_BulletSpeedMultiplier = 100.0f;
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
