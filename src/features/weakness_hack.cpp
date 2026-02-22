#include "weakness_hack.h"
#include "settings.h"
#include <MinHook.h>
#include <cstdio>
#include <cstring>

// IL2CPP string helper
static void* GetWeaknessString() {
    if (!g_WeaknessString && il2cpp_string_new) {
        g_WeaknessString = il2cpp_string_new("Monster_Weakness");
    }
    
    if (g_WeaknessString) {
        __try {
            int len = *(int*)((char*)g_WeaknessString + 0x10);
            if (len != 16) {
                if (il2cpp_string_new) {
                    g_WeaknessString = il2cpp_string_new("Monster_Weakness");
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            if (il2cpp_string_new) {
                g_WeaknessString = il2cpp_string_new("Monster_Weakness");
            }
        }
    }
    
    return g_WeaknessString;
}

static bool IsWeaknessTag(void* str) {
    if (!str) return false;
    
    __try {
        int len = *(int*)((char*)str + 0x10);
        if (len <= 0 || len > 50) return false;
        
        wchar_t* chars = (wchar_t*)((char*)str + 0x14);
        
        for (int i = 0; i <= len - 8; i++) {
            if ((chars[i] == L'W' || chars[i] == L'w') &&
                (chars[i+1] == L'e') &&
                (chars[i+2] == L'a') &&
                (chars[i+3] == L'k') &&
                (chars[i+4] == L'n') &&
                (chars[i+5] == L'e') &&
                (chars[i+6] == L's') &&
                (chars[i+7] == L's')) {
                return true;
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    
    return false;
}

// Check whether hitpart should be overridden
static bool ShouldModifyHitpart(void* str) {
    if (!str) return false;
    if (IsWeaknessTag(str)) return false;
    
    __try {
        int len = *(int*)((char*)str + 0x10);
        if (len <= 0 || len > 50) return false;
        
        wchar_t* chars = (wchar_t*)((char*)str + 0x14);
        
        if (len == 1 && chars[0] == L'0') return false;
        
        if (len == 8 && 
            chars[0] == L'U' && chars[1] == L'n' && chars[2] == L't' && chars[3] == L'a' &&
            chars[4] == L'g' && chars[5] == L'g' && chars[6] == L'e' && chars[7] == L'd') {
            return false;
        }
        
        if (len >= 8 && 
            chars[0] == L'M' && chars[1] == L'o' && chars[2] == L'n' && chars[3] == L's' &&
            chars[4] == L't' && chars[5] == L'e' && chars[6] == L'r' && chars[7] == L'_') {
            return true;
        }
        
        return false;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// SClientHitInfo ctor hook
void HookedSClientHitInfoCtor(void* thisPtr, int vitid, void* luago, void* tran, bool islife,
    Vector3* hitPos, Vector3* hitNormal, void* hitTrans, Vector3* hforward, void* hpart, void* spart) {
    
    void* finalHpart = hpart;
    
    if (!g_ShuttingDown.load(std::memory_order_acquire) && 
        g_HooksInstalled.load(std::memory_order_acquire) &&
        g_WeaknessHack && hpart && ShouldModifyHitpart(hpart)) {
        void* weakStr = GetWeaknessString();
        if (weakStr) {
            finalHpart = weakStr;
        }
    }
    
    if (g_OriginalSClientHitInfoCtor) {
        g_OriginalSClientHitInfoCtor(thisPtr, vitid, luago, tran, islife, hitPos, hitNormal, hitTrans, hforward, finalHpart, spart);
    }
}

// CartoonData.SetSkilllRay hook
void HookedCartoonDataSetSkilllRay(void* thisPtr, void* rayHit) {
    if (g_ShuttingDown.load(std::memory_order_acquire) || !g_HooksInstalled.load(std::memory_order_acquire)) {
        if (g_OriginalCartoonDataSetSkilllRay) {
            g_OriginalCartoonDataSetSkilllRay(thisPtr, rayHit);
        }
        return;
    }
    
    if (g_WeaknessHack && rayHit) {
        __try {
            void** hitpartPtr = (void**)((char*)rayHit + 0x20);
            void* hitpart = *hitpartPtr;
            
            if (hitpart && ShouldModifyHitpart(hitpart)) {
                void* weakStr = GetWeaknessString();
                if (weakStr) {
                    *hitpartPtr = weakStr;
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    
    if (g_OriginalCartoonDataSetSkilllRay) {
        g_OriginalCartoonDataSetSkilllRay(thisPtr, rayHit);
    }
}

// CartoonData.PacketSkillRay hook
void HookedCartoonDataPacketSkillRay(void* thisPtr, void* lsthit) {
    if (g_ShuttingDown.load(std::memory_order_acquire) || !g_HooksInstalled.load(std::memory_order_acquire)) {
        if (g_OriginalCartoonDataPacketSkillRay) {
            g_OriginalCartoonDataPacketSkillRay(thisPtr, lsthit);
        }
        return;
    }
    
    if (g_WeaknessHack && lsthit) {
        void* weakStr = GetWeaknessString();
        if (weakStr) {
            __try {
                void* items = *(void**)((char*)lsthit + 0x10);
                int size = *(int*)((char*)lsthit + 0x18);
                
                if (items && size > 0 && size < 100) {
                    for (int i = 0; i < size; i++) {
                        char* rayHit = (char*)items + 0x20 + i * 0x40;
                        void** hitpartPtr = (void**)(rayHit + 0x20);
                        void* hitpart = *hitpartPtr;
                        
                        if (hitpart && ShouldModifyHitpart(hitpart)) {
                            *hitpartPtr = weakStr;
                        }
                    }
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }
    }
    
    if (g_OriginalCartoonDataPacketSkillRay) {
        g_OriginalCartoonDataPacketSkillRay(thisPtr, lsthit);
    }
}

bool InstallWeaknessHooks() {
    if (!g_CartoonDataSetSkilllRayAddr || !g_CartoonDataPacketSkillRayAddr || !g_SClientHitInfoCtorAddr) {
        printf("[GFR Mod] Weakness hack addresses not initialized\n");
        return false;
    }
    
    if (MH_CreateHook(g_SClientHitInfoCtorAddr, (void*)HookedSClientHitInfoCtor,
                      (void**)&g_OriginalSClientHitInfoCtor) != MH_OK) {
        printf("[GFR Mod] Failed to create SClientHitInfoCtor hook\n");
        return false;
    }
    
    if (MH_CreateHook(g_CartoonDataSetSkilllRayAddr, (void*)HookedCartoonDataSetSkilllRay,
                      (void**)&g_OriginalCartoonDataSetSkilllRay) != MH_OK) {
        printf("[GFR Mod] Failed to create CartoonDataSetSkilllRay hook\n");
        return false;
    }
    
    if (MH_CreateHook(g_CartoonDataPacketSkillRayAddr, (void*)HookedCartoonDataPacketSkillRay,
                      (void**)&g_OriginalCartoonDataPacketSkillRay) != MH_OK) {
        printf("[GFR Mod] Failed to create CartoonDataPacketSkillRay hook\n");
        return false;
    }
    
    printf("[GFR Mod] Weakness hooks installed\n");
    return true;
}

void RemoveWeaknessHooks() {
    if (g_SClientHitInfoCtorAddr) MH_RemoveHook(g_SClientHitInfoCtorAddr);
    if (g_CartoonDataSetSkilllRayAddr) MH_RemoveHook(g_CartoonDataSetSkilllRayAddr);
    if (g_CartoonDataPacketSkillRayAddr) MH_RemoveHook(g_CartoonDataPacketSkillRayAddr);
}
