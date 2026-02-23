#include "esp.h"
#include "settings.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>

std::vector<ESPObject> g_ESPObjects;
float g_ViewMatrix[16] = {0};
bool g_ViewMatrixValid = false;

namespace {
Il2CppMethod* g_ObjectGetNameMethod = nullptr;
bool g_ObjectGetNameMethodResolved = false;

struct FightTypeRuntime {
    bool resolvedFromGame = false;
    bool fallbackLogged = false;
    DWORD lastResolveAttemptMs = 0;
    bool hasNpcEvent = false;
    int npcEvent = 0;
    std::vector<int> rewardTypes;
    std::vector<int> serviceNpcTypes;
} g_FightTypeRuntime;

bool ContainsFightType(const std::vector<int>& values, int fightType) {
    for (int value : values) {
        if (value == fightType) return true;
    }
    return false;
}

bool TryReadFightTypeField(Il2CppClass* fightTypeClass, const char* fieldName, int* outValue) {
    if (!fightTypeClass || !fieldName || !outValue || !il2cpp_class_get_field_from_name || !il2cpp_field_static_get_value) {
        return false;
    }

    void* field = il2cpp_class_get_field_from_name(fightTypeClass, fieldName);
    if (!field) return false;

    __try {
        int value = 0;
        il2cpp_field_static_get_value(field, &value);
        *outValue = value;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

Il2CppClass* FindFightTypeClassByFields(Il2CppImage* imgCSharp) {
    if (!imgCSharp || !il2cpp_image_get_class_count || !il2cpp_image_get_class || !il2cpp_class_get_field_from_name) {
        return nullptr;
    }

    size_t classCount = il2cpp_image_get_class_count(imgCSharp);
    for (size_t i = 0; i < classCount; i++) {
        Il2CppClass* klass = nullptr;
        __try {
            klass = il2cpp_image_get_class(imgCSharp, i);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            klass = nullptr;
        }
        if (!klass) continue;

        // Signature field from FightType enum in current dump.
        if (il2cpp_class_get_field_from_name(klass, "NWARRIOR_NPC_EVENT")) {
            return klass;
        }
    }

    return nullptr;
}

void ResolveFightTypesFromGame() {
    if (g_FightTypeRuntime.resolvedFromGame) return;

    DWORD now = GetTickCount();
    if (g_FightTypeRuntime.lastResolveAttemptMs != 0 &&
        (now - g_FightTypeRuntime.lastResolveAttemptMs) < 2000) {
        return;
    }
    g_FightTypeRuntime.lastResolveAttemptMs = now;

    Il2CppImage* imgCSharp = FindImage("Assembly-CSharp");
    if (!imgCSharp || !il2cpp_class_from_name) {
        if (!g_FightTypeRuntime.fallbackLogged) {
            printf("[GFR Mod] FightType resolver: Assembly-CSharp not ready, dynamic mapping deferred\n");
            g_FightTypeRuntime.fallbackLogged = true;
        }
        return;
    }

    Il2CppClass* fightTypeClass = il2cpp_class_from_name(imgCSharp, "", "FightType");
    if (!fightTypeClass) {
        // Some dumps show this enum as ServerDefine.FightType.
        fightTypeClass = il2cpp_class_from_name(imgCSharp, "ServerDefine", "FightType");
    }
    if (!fightTypeClass) {
        // Some IL2CPP builds expose nested/qualified names in the type name field.
        fightTypeClass = il2cpp_class_from_name(imgCSharp, "", "ServerDefine.FightType");
    }
    if (!fightTypeClass) {
        fightTypeClass = FindFightTypeClassByFields(imgCSharp);
    }
    if (!fightTypeClass) {
        if (!g_FightTypeRuntime.fallbackLogged) {
            printf("[GFR Mod] FightType resolver: enum class not found, dynamic mapping deferred\n");
            g_FightTypeRuntime.fallbackLogged = true;
        }
        return;
    }

    if (il2cpp_class_get_namespace && il2cpp_class_get_name) {
        const char* ns = il2cpp_class_get_namespace(fightTypeClass);
        const char* cn = il2cpp_class_get_name(fightTypeClass);
        printf("[GFR Mod] FightType resolver: using class %s.%s\n",
            (ns && ns[0]) ? ns : "<global>", cn ? cn : "<unknown>");
    }

    g_FightTypeRuntime.hasNpcEvent = false;
    g_FightTypeRuntime.rewardTypes.clear();
    g_FightTypeRuntime.serviceNpcTypes.clear();

    int resolvedCount = 0;

    int npcEventValue = 0;
    if (TryReadFightTypeField(fightTypeClass, "NWARRIOR_NPC_EVENT", &npcEventValue)) {
        resolvedCount++;
        g_FightTypeRuntime.npcEvent = npcEventValue;
        g_FightTypeRuntime.hasNpcEvent = true;
        printf("[GFR Mod] FightType NWARRIOR_NPC_EVENT = 0x%08X (%d)\n", npcEventValue, npcEventValue);
    }

    const std::array<const char*, 12> rewardNames = {
        "NWARRIOR_NPC_ITEMBOX",
        "NWARRIOR_NPC_PASSBOX",
        "NWARRIOR_NPC_GOLDENCUP",
        "NWARRIOR_NPC_LOCKEDBOX",
        "NWARRIOR_NPC_MAGICBOX",
        "NWARRIOR_NPC_INITBOX",
        "NWARRIOR_NPC_ROOMCHALLENGE",
        "NWARRIOR_NPC_LIMITGOLDENCUP",
        "NWARRIOR_NPC_RAREGOLDENCUP",
        "NWARRIOR_NPC_EXCHANGEGOLDENCUP",
        "NWARRIOR_NPC_RELICROLLBOX",
        "NWARRIOR_NPC_RELICLOTTERY"
    };
    for (size_t i = 0; i < rewardNames.size(); i++) {
        int value = 0;
        if (TryReadFightTypeField(fightTypeClass, rewardNames[i], &value)) {
            resolvedCount++;
            g_FightTypeRuntime.rewardTypes.push_back(value);
            printf("[GFR Mod] FightType %s = 0x%08X (%d)\n", rewardNames[i], value, value);
        }
    }

    const std::array<const char*, 12> serviceNpcNames = {
        "NWARRIOR_NPC_SHOP",
        "NWARRIOR_NPC_SMITH",
        "NWARRIOR_NPC_CAMPFIRE",
        "NWARRIOR_NPC_BENEDICTION",
        "NWARRIOR_NPC_GSCASHSHOP",
        "NWARRIOR_NPC_RELIC",
        "NWARRIOR_NPC_WEAPONSTORE",
        "NWARRIOR_NPC_TASKNPC",
        "NWARRIOR_NPC_PETSHOP",
        "NWARRIOR_NPC_WANDSHOP",
        "NWARRIOR_NPC_DICESHOP",
        "NWARRIOR_NPC_S7SHOP"
    };
    for (size_t i = 0; i < serviceNpcNames.size(); i++) {
        int value = 0;
        if (TryReadFightTypeField(fightTypeClass, serviceNpcNames[i], &value)) {
            resolvedCount++;
            g_FightTypeRuntime.serviceNpcTypes.push_back(value);
            printf("[GFR Mod] FightType %s = 0x%08X (%d)\n", serviceNpcNames[i], value, value);
        }
    }

    g_FightTypeRuntime.resolvedFromGame = (resolvedCount > 0);
    if (g_FightTypeRuntime.resolvedFromGame) {
        g_FightTypeRuntime.fallbackLogged = false;
        printf("[GFR Mod] FightType resolver: loaded %d enum values from game metadata\n", resolvedCount);
    } else {
        if (!g_FightTypeRuntime.fallbackLogged) {
            printf("[GFR Mod] FightType resolver: no enum values loaded, dynamic mapping unavailable\n");
            g_FightTypeRuntime.fallbackLogged = true;
        }
    }
}

bool IsSecretWallSid(int sid) {
    return sid == SID_SECRET_WALL ||
           sid == SID_SECRET_WALL_2 ||
           sid == SID_SECRET_WALL_3 ||
           sid == SID_SECRET_WALL_4 ||
           sid == SID_SECRET_WALL_5;
}

bool IsSecretPortalSid(int sid) {
    return sid == SID_SECRET_PORTAL ||
           sid == SID_SECRET_PORTAL_2 ||
           sid == SID_SECRET_PORTAL_3;
}

bool IsRewardNpcFightType(int fightType) {
    ResolveFightTypesFromGame();
    return ContainsFightType(g_FightTypeRuntime.rewardTypes, fightType);
}

bool IsServiceNpcFightType(int fightType) {
    ResolveFightTypesFromGame();
    return ContainsFightType(g_FightTypeRuntime.serviceNpcTypes, fightType);
}

int GetNpcEventFightType() {
    ResolveFightTypesFromGame();
    return g_FightTypeRuntime.hasNpcEvent ? g_FightTypeRuntime.npcEvent : -1;
}

const char* GetTypeFallbackName(ESPType type) {
    switch (type) {
    case ESPType::SecretWall: return "Secret Room Wall";
    case ESPType::SecretPortal: return "Secret Room Portal";
    case ESPType::TreasureBox: return "Treasure Box";
    case ESPType::EventBox: return "Event Box";
    case ESPType::NPC: return "NPC";
    default: return "Object";
    }
}

bool IsNumericLikeName(const char* text) {
    if (!text || !text[0]) return true;

    // If name is like "12345" or "12345(Clone)" treat as non-meaningful.
    char buf[128];
    strncpy_s(buf, sizeof(buf), text, _TRUNCATE);
    char* clonePos = strstr(buf, "(Clone)");
    if (clonePos) {
        *clonePos = '\0';
    }

    bool hasDigit = false;
    for (int i = 0; buf[i]; i++) {
        char c = buf[i];
        if (c >= '0' && c <= '9') {
            hasDigit = true;
            continue;
        }
        if (c == ' ' || c == '_' || c == '-' || c == '.') {
            continue;
        }
        return false;
    }
    return hasDigit;
}

void ResolveObjectGetNameMethod() {
    if (g_ObjectGetNameMethodResolved) return;
    g_ObjectGetNameMethodResolved = true;

    if (!il2cpp_class_from_name || !il2cpp_class_get_method_from_name) return;

    Il2CppImage* imgCore = FindImage("UnityEngine.CoreModule");
    if (!imgCore) return;

    Il2CppClass* objectClass = il2cpp_class_from_name(imgCore, "UnityEngine", "Object");
    if (!objectClass) return;

    g_ObjectGetNameMethod = il2cpp_class_get_method_from_name(objectClass, "get_name", 0);
}

bool TryIl2CppStringToAnsi(Il2CppObject* strObj, char* outBuf, size_t outBufSize) {
    if (!strObj || !outBuf || outBufSize < 2) return false;
    outBuf[0] = '\0';

    __try {
        // IL2CPP String layout: [object header 0x10] [int32 length @ 0x10] [wchar_t[] chars @ 0x14]
        int len = *(int*)((char*)strObj + 0x10);
        if (len <= 0) return false;
        wchar_t* chars = (wchar_t*)((char*)strObj + 0x14);

        int maxChars = (int)outBufSize - 1;
        if (len > maxChars) len = maxChars;

        for (int i = 0; i < len; i++) {
            wchar_t wc = chars[i];
            if (wc >= 32 && wc <= 126) {
                outBuf[i] = (char)wc;
            } else if (wc == 9 || wc == 10 || wc == 13) {
                outBuf[i] = ' ';
            } else {
                outBuf[i] = '?';
            }
        }
        outBuf[len] = '\0';
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        outBuf[0] = '\0';
        return false;
    }
}

std::string BuildESPName(Il2CppObject* obj, ESPType type) {
    ResolveObjectGetNameMethod();
    if (!g_ObjectGetNameMethod || !il2cpp_runtime_invoke) {
        return GetTypeFallbackName(type);
    }

    void* exception = nullptr;
    Il2CppObject* nameObj = il2cpp_runtime_invoke(g_ObjectGetNameMethod, obj, nullptr, &exception);
    if (exception || !nameObj) {
        return GetTypeFallbackName(type);
    }

    char nameBuf[128];
    if (TryIl2CppStringToAnsi(nameObj, nameBuf, sizeof(nameBuf))) {
        if (nameBuf[0] != '\0' && !IsNumericLikeName(nameBuf)) {
            return std::string(nameBuf);
        }
    }

    return GetTypeFallbackName(type);
}
} // namespace

bool WorldToScreen(const Vector3& worldPos, Vector3& screenPos, int screenWidth, int screenHeight) {
    AcquireSRWLockShared(&g_MatrixLock);
    if (!g_ViewMatrixValid) {
        ReleaseSRWLockShared(&g_MatrixLock);
        return false;
    }
    
    float* m = g_ViewMatrix;
    
    float clipX = worldPos.x * m[0] + worldPos.y * m[4] + worldPos.z * m[8] + m[12];
    float clipY = worldPos.x * m[1] + worldPos.y * m[5] + worldPos.z * m[9] + m[13];
    float clipW = worldPos.x * m[3] + worldPos.y * m[7] + worldPos.z * m[11] + m[15];
    
    ReleaseSRWLockShared(&g_MatrixLock);
    
    if (clipW < 0.1f) return false;
    
    float ndcX = clipX / clipW;
    float ndcY = clipY / clipW;
    
    screenPos.x = (screenWidth * 0.5f) * (1.0f + ndcX);
    screenPos.y = (screenHeight * 0.5f) * (1.0f - ndcY);
    screenPos.z = clipW;
    
    return true;
}

void MultiplyMatrix(const float* a, const float* b, float* result) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i * 4 + j] = 
                a[i * 4 + 0] * b[0 * 4 + j] +
                a[i * 4 + 1] * b[1 * 4 + j] +
                a[i * 4 + 2] * b[2 * 4 + j] +
                a[i * 4 + 3] * b[3 * 4 + j];
        }
    }
}

void UpdateViewMatrix() {
    if (!g_GetMainCameraComDirect || !g_GetWorldToCameraMatrix || !g_GetProjectionMatrix) {
        AcquireSRWLockExclusive(&g_MatrixLock);
        g_ViewMatrixValid = false;
        ReleaseSRWLockExclusive(&g_MatrixLock);
        return;
    }

    void* camera = g_GetMainCameraComDirect();
    if (!camera) {
        AcquireSRWLockExclusive(&g_MatrixLock);
        g_ViewMatrixValid = false;
        ReleaseSRWLockExclusive(&g_MatrixLock);
        return;
    }

    Matrix4x4 viewMatrix = {0}, projMatrix = {0};

    __try {
        g_GetWorldToCameraMatrix(camera, &viewMatrix);
        g_GetProjectionMatrix(camera, &projMatrix);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        AcquireSRWLockExclusive(&g_MatrixLock);
        g_ViewMatrixValid = false;
        ReleaseSRWLockExclusive(&g_MatrixLock);
        return;
    }

    float vpMatrix[16];
    MultiplyMatrix(viewMatrix.m, projMatrix.m, vpMatrix);
    
    AcquireSRWLockExclusive(&g_MatrixLock);
    memcpy(g_ViewMatrix, vpMatrix, sizeof(g_ViewMatrix));
    g_ViewMatrixValid = true;
    ReleaseSRWLockExclusive(&g_MatrixLock);
}

void UpdateESPObjects() {
    if (!g_PlayerDictField || !g_MainCtrlField || !g_GetPositionInjected) {
        return;
    }
    
    int mainCtrl = 0;
    il2cpp_field_static_get_value(g_MainCtrlField, &mainCtrl);
    if (mainCtrl == 0) {
        AcquireSRWLockExclusive(&g_ESPLock);
        g_ESPObjects.clear();
        ReleaseSRWLockExclusive(&g_ESPLock);
        return;
    }
    
    Il2CppObject* playerDict = nullptr;
    il2cpp_field_static_get_value(g_PlayerDictField, &playerDict);
    if (!playerDict) {
        AcquireSRWLockExclusive(&g_ESPLock);
        g_ESPObjects.clear();
        ReleaseSRWLockExclusive(&g_ESPLock);
        return;
    }
    
    auto entries = *(Il2CppObject**)((char*)playerDict + 0x18);
    int dictCount = *(int*)((char*)playerDict + 0x20);
    if (!entries || dictCount <= 0 || dictCount > 500) {
        AcquireSRWLockExclusive(&g_ESPLock);
        g_ESPObjects.clear();
        ReleaseSRWLockExclusive(&g_ESPLock);
        return;
    }
    
    std::vector<ESPObject> newObjects;
    Vector3 playerPos = {0, 0, 0};
    for (int i = 0; i < dictCount + 50 && i < 500; i++) {
        char* entryBase = (char*)entries + 0x20 + i * 24;
        int hashCode = *(int*)entryBase;
        if (hashCode < 0) continue;
        
        int key = *(int*)(entryBase + 0x8);
        if (key != mainCtrl) continue;
        
        auto playerObj = *(Il2CppObject**)(entryBase + 0x10);
        if (!playerObj) continue;
        
        auto playerTrans = *(Il2CppObject**)((char*)playerObj + OFFSET_GAMETRANS);
        if (playerTrans) {
            g_GetPositionInjected(playerTrans, &playerPos);
        }
        break;
    }
    
    for (int i = 0; i < dictCount + 50 && i < 500; i++) {
        char* entryBase = (char*)entries + 0x20 + i * 24;
        int hashCode = *(int*)entryBase;
        if (hashCode < 0) continue;
        
        auto playerObj = *(Il2CppObject**)(entryBase + 0x10);
        if (!playerObj) continue;
        
        int fightType = *(int*)((char*)playerObj + OFFSET_FIGHTTYPE);
        int sid = *(int*)((char*)playerObj + OFFSET_SID);
        
        bool isSecretWall = IsSecretWallSid(sid);
        bool isSecretPortal = IsSecretPortalSid(sid);
        bool isTreasureBox = IsRewardNpcFightType(fightType);
        bool isEventBox = (fightType == GetNpcEventFightType());
        bool isGenericNpc = g_ShowNPCESP && IsServiceNpcFightType(fightType);

        if (!isSecretWall && !isSecretPortal && !isTreasureBox && !isEventBox && !isGenericNpc) continue;

        auto gameTrans = *(Il2CppObject**)((char*)playerObj + OFFSET_GAMETRANS);
        if (!gameTrans) continue;

        Vector3 pos;
        g_GetPositionInjected(gameTrans, &pos);

        ESPObject obj;
        obj.worldPos = pos;
        if (isSecretWall) obj.type = ESPType::SecretWall;
        else if (isSecretPortal) obj.type = ESPType::SecretPortal;
        else if (isTreasureBox) obj.type = ESPType::TreasureBox;
        else if (isEventBox) obj.type = ESPType::EventBox;
        else obj.type = ESPType::NPC;
        obj.displayName = BuildESPName(gameTrans, obj.type);
        
        float dx = pos.x - playerPos.x;
        float dy = pos.y - playerPos.y;
        float dz = pos.z - playerPos.z;
        obj.distance = sqrtf(dx*dx + dy*dy + dz*dz);
        
        newObjects.push_back(obj);
    }
    
    AcquireSRWLockExclusive(&g_ESPLock);
    g_ESPObjects = std::move(newObjects);
    ReleaseSRWLockExclusive(&g_ESPLock);
}
