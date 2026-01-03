#pragma once
#include "../game/game.h"

// Hooked functions
void HookedSClientHitInfoCtor(void* thisPtr, int vitid, void* luago, void* tran, bool islife,
    Vector3* hitPos, Vector3* hitNormal, void* hitTrans, Vector3* hforward, void* hpart, void* spart);
void HookedCartoonDataSetSkilllRay(void* thisPtr, void* rayHit);
void HookedCartoonDataPacketSkillRay(void* thisPtr, void* lsthit);

bool InstallWeaknessHooks();
void RemoveWeaknessHooks();
