#include "weakness_hack.h"
#include "settings.h"
#include <MinHook.h>
#include <cstdio>
#include <cstring>

// IL2CPP String 구조
// offset 0x10: int length
// offset 0x14: wchar_t[] chars

// 문자열 유효성 체크 및 재생성
static void* GetWeaknessString() {
    // 문자열이 없거나 유효하지 않으면 재생성
    if (!g_WeaknessString && il2cpp_string_new) {
        g_WeaknessString = il2cpp_string_new("Monster_Weakness");
    }
    
    // 유효성 체크
    if (g_WeaknessString) {
        __try {
            int len = *(int*)((char*)g_WeaknessString + 0x10);
            if (len != 16) {  // "Monster_Weakness" 길이
                // 무효화됨, 재생성
                if (il2cpp_string_new) {
                    g_WeaknessString = il2cpp_string_new("Monster_Weakness");
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            // 접근 불가, 재생성
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
        
        // "Weakness" 또는 "weakness" 포함 여부 체크
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

// IL2CPP 문자열인지 확인하고 수정해야 하는지 체크
static bool ShouldModifyHitpart(void* str) {
    if (!str) return false;
    if (IsWeaknessTag(str)) return false;  // 이미 약점이면 수정 안함
    
    __try {
        int len = *(int*)((char*)str + 0x10);
        if (len <= 0 || len > 50) return false;
        
        wchar_t* chars = (wchar_t*)((char*)str + 0x14);
        
        // "0"이면 수정 안함 (빗나간 샷)
        if (len == 1 && chars[0] == L'0') return false;
        
        // "Untagged"면 수정 안함
        if (len == 8 && 
            chars[0] == L'U' && chars[1] == L'n' && chars[2] == L't' && chars[3] == L'a' &&
            chars[4] == L'g' && chars[5] == L'g' && chars[6] == L'e' && chars[7] == L'd') {
            return false;
        }
        
        // Monster_로 시작하는 태그만 수정 (Monster_UpperBody, Monster_LowerBody 등)
        if (len >= 8 && 
            chars[0] == L'M' && chars[1] == L'o' && chars[2] == L'n' && chars[3] == L's' &&
            chars[4] == L't' && chars[5] == L'e' && chars[6] == L'r' && chars[7] == L'_') {
            return true;
        }
        
        return false;  // 그 외에는 수정 안함
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// SClientHitInfo 생성자 후킹 - hitpart 수정 (핵심!)
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

// CartoonData.SetSkilllRay 후킹 - hitpart 수정
void HookedCartoonDataSetSkilllRay(void* thisPtr, void* rayHit) {
    if (g_ShuttingDown.load(std::memory_order_acquire) || !g_HooksInstalled.load(std::memory_order_acquire)) {
        if (g_OriginalCartoonDataSetSkilllRay) {
            g_OriginalCartoonDataSetSkilllRay(thisPtr, rayHit);
        }
        return;
    }
    
    // hitpart 수정 (SkillRayHit 값 타입, hitpart 오프셋 0x20)
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
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            // 예외 무시
        }
    }
    
    if (g_OriginalCartoonDataSetSkilllRay) {
        g_OriginalCartoonDataSetSkilllRay(thisPtr, rayHit);
    }
}

// CartoonData.PacketSkillRay 후킹 - 서버로 보내기 직전에 List<SkillRayHit> 수정
void HookedCartoonDataPacketSkillRay(void* thisPtr, void* lsthit) {
    if (g_ShuttingDown.load(std::memory_order_acquire) || !g_HooksInstalled.load(std::memory_order_acquire)) {
        if (g_OriginalCartoonDataPacketSkillRay) {
            g_OriginalCartoonDataPacketSkillRay(thisPtr, lsthit);
        }
        return;
    }
    
    // List<SkillRayHit>의 모든 항목 수정
    if (g_WeaknessHack && lsthit) {
        void* weakStr = GetWeaknessString();
        if (weakStr) {
            __try {
                // List<T> 구조: _items at 0x10, _size at 0x18
                void* items = *(void**)((char*)lsthit + 0x10);
                int size = *(int*)((char*)lsthit + 0x18);
                
                if (items && size > 0 && size < 100) {
                    for (int i = 0; i < size; i++) {
                        // SkillRayHit는 값 타입, 크기 0x40 (패딩 포함)
                        // Array 시작 오프셋 0x20, hitpart 오프셋 0x20
                        char* rayHit = (char*)items + 0x20 + i * 0x40;
                        void** hitpartPtr = (void**)(rayHit + 0x20);
                        void* hitpart = *hitpartPtr;
                        
                        if (hitpart && ShouldModifyHitpart(hitpart)) {
                            *hitpartPtr = weakStr;
                        }
                    }
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                // 예외 무시
            }
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
    
    // SClientHitInfo.ctor hook (핵심!)
    if (MH_CreateHook(g_SClientHitInfoCtorAddr, (void*)HookedSClientHitInfoCtor,
                      (void**)&g_OriginalSClientHitInfoCtor) != MH_OK) {
        printf("[GFR Mod] Failed to create SClientHitInfoCtor hook\n");
        return false;
    }
    
    // CartoonData.SetSkilllRay hook
    if (MH_CreateHook(g_CartoonDataSetSkilllRayAddr, (void*)HookedCartoonDataSetSkilllRay,
                      (void**)&g_OriginalCartoonDataSetSkilllRay) != MH_OK) {
        printf("[GFR Mod] Failed to create CartoonDataSetSkilllRay hook\n");
        return false;
    }
    
    // CartoonData.PacketSkillRay hook (서버 전송 직전)
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
