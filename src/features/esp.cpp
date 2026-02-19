#include "esp.h"
#include "settings.h"
#include <cmath>
#include <cstdio>
#include <cstring>

std::vector<ESPObject> g_ESPObjects;
float g_ViewMatrix[16] = {0};
bool g_ViewMatrixValid = false;

namespace {
Il2CppMethod* g_ObjectGetNameMethod = nullptr;
bool g_ObjectGetNameMethodResolved = false;

const char* GetTypeFallbackName(ESPType type) {
    switch (type) {
    case ESPType::SecretWall: return "Secret Room Wall";
    case ESPType::SecretPortal: return "Secret Room Portal";
    case ESPType::TreasureBox: return "Treasure Box";
    case ESPType::EventBox: return "Event Box";
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
        
        bool isSecretWall = (fightType == FIGHTTYPE_OBSTACLE_NORMAL &&
            (sid == SID_SECRET_WALL || sid == SID_SECRET_WALL_2 || sid == SID_SECRET_WALL_3 || sid == SID_SECRET_WALL_4 || sid == SID_SECRET_WALL_5));
        bool isSecretPortal = (fightType == FIGHTTYPE_NPC_TRANSFER &&
            (sid == SID_SECRET_PORTAL || sid == SID_SECRET_PORTAL_2 || sid == SID_SECRET_PORTAL_3));
        bool isTreasureBox = (fightType == FIGHTTYPE_NPC_TREASUREBOX);
        bool isEventBox = (fightType == FIGHTTYPE_NPC_EVENT);

        if (!isSecretWall && !isSecretPortal && !isTreasureBox && !isEventBox) continue;

        auto gameTrans = *(Il2CppObject**)((char*)playerObj + OFFSET_GAMETRANS);
        if (!gameTrans) continue;

        Vector3 pos;
        g_GetPositionInjected(gameTrans, &pos);

        ESPObject obj;
        obj.worldPos = pos;
        if (isSecretWall) obj.type = ESPType::SecretWall;
        else if (isSecretPortal) obj.type = ESPType::SecretPortal;
        else if (isTreasureBox) obj.type = ESPType::TreasureBox;
        else obj.type = ESPType::EventBox;
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
