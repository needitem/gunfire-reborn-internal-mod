#pragma once
#include "../game/game.h"

extern GetFOV_t g_OriginalGetFOV;
extern void* g_GetFOVAddr;

float HookedGetFOV(void* camera);
