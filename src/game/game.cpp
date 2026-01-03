#include "game.h"

// Cached methods
Il2CppMethod* g_GetMonsters = nullptr;
Il2CppMethod* g_GetPlayer = nullptr;
Il2CppMethod* g_GetMaxBullet = nullptr;
Il2CppMethod* g_GetCurBullet = nullptr;
Il2CppMethod* g_SetCurBullet = nullptr;
Il2CppMethod* g_SetClientCurBullet = nullptr;
Il2CppMethod* g_GetSpeed = nullptr;
Il2CppMethod* g_SetSpeed = nullptr;
Il2CppMethod* g_GetJumpHeight = nullptr;
Il2CppMethod* g_SetJumpHeight = nullptr;
Il2CppMethod* g_OneGPU = nullptr;
Il2CppMethod* g_StartGPU = nullptr;
Il2CppMethod* g_GetWeakTrans = nullptr;
Il2CppMethod* g_GetPosition = nullptr;
Il2CppMethod* g_GetTransform = nullptr;
Il2CppMethod* g_GetForward = nullptr;
Il2CppMethod* g_GetMainCameraCom = nullptr;
Il2CppMethod* g_ListGetCount = nullptr;
Il2CppMethod* g_ListGetItem = nullptr;
Il2CppMethod* g_SetNoCostBullet = nullptr;
Il2CppMethod* g_GetSpecialWeakTrans = nullptr;
Il2CppMethod* g_GetWeakTransByTag = nullptr;

// Cached classes/fields
Il2CppClass* g_NewPlayerManager = nullptr;
Il2CppClass* g_NewItemProp = nullptr;
Il2CppClass* g_PlayerProp = nullptr;
Il2CppClass* g_DropManager = nullptr;
Il2CppClass* g_OCBodyPart = nullptr;
Il2CppClass* g_GMStateManager = nullptr;
void* g_MainCtrlField = nullptr;
void* g_PlayerDictField = nullptr;

// Direct function pointers
GetPositionInjected_t g_GetPositionInjected = nullptr;
SetPositionInjected_t g_SetPositionInjected = nullptr;
GetMatrix_t g_GetWorldToCameraMatrix = nullptr;
GetMatrix_t g_GetProjectionMatrix = nullptr;
GetMainCameraComDirect_t g_GetMainCameraComDirect = nullptr;
GetWarCash_t g_GetWarCash = nullptr;
SetWarCash_t g_SetWarCash = nullptr;
GetPlayerProp_t g_GetPlayerPropFunc = nullptr;
GetFOV_t g_GetFOV = nullptr;
SetFOV_t g_SetFOV = nullptr;

// Weakness hit hack
CartoonDataSetSkilllRay_t g_OriginalCartoonDataSetSkilllRay = nullptr;
CartoonDataPacketSkillRay_t g_OriginalCartoonDataPacketSkillRay = nullptr;
SClientHitInfoCtor_t g_OriginalSClientHitInfoCtor = nullptr;

void* g_CartoonDataSetSkilllRayAddr = nullptr;
void* g_CartoonDataPacketSkillRayAddr = nullptr;
void* g_SClientHitInfoCtorAddr = nullptr;

void* g_WeaknessString = nullptr;
void* g_SpecialWeaknessString = nullptr;

bool InitGame() {
    auto imgCSharp = FindImage("Assembly-CSharp");
    auto imgCore = FindImage("UnityEngine.CoreModule");
    if (!imgCSharp || !imgCore) return false;
    
    g_NewPlayerManager = il2cpp_class_from_name(imgCSharp, "", "NewPlayerManager");
    g_NewItemProp = il2cpp_class_from_name(imgCSharp, "", "NewItemProp");
    g_PlayerProp = il2cpp_class_from_name(imgCSharp, "", "PlayerProp");
    g_DropManager = il2cpp_class_from_name(imgCSharp, "", "DropManager");
    g_OCBodyPart = il2cpp_class_from_name(imgCSharp, "", "OCBodyPart");
    auto Transform = il2cpp_class_from_name(imgCore, "UnityEngine", "Transform");
    
    if (!g_NewPlayerManager) return false;
    
    g_GetMonsters = il2cpp_class_get_method_from_name(g_NewPlayerManager, "GetMonsters", 0);
    g_GetPlayer = il2cpp_class_get_method_from_name(g_NewPlayerManager, "GetPlayer", 1);
    
    if (g_NewItemProp) {
        g_GetMaxBullet = il2cpp_class_get_method_from_name(g_NewItemProp, "get_MaxBullet", 0);
        g_GetCurBullet = il2cpp_class_get_method_from_name(g_NewItemProp, "get_CurBullet", 0);
        g_SetCurBullet = il2cpp_class_get_method_from_name(g_NewItemProp, "set_CurBullet", 1);
        g_SetClientCurBullet = il2cpp_class_get_method_from_name(g_NewItemProp, "set_ClientCurBullet", 1);
    }
    
    if (g_PlayerProp) {
        g_GetSpeed = il2cpp_class_get_method_from_name(g_PlayerProp, "get_Speed", 0);
        g_SetSpeed = il2cpp_class_get_method_from_name(g_PlayerProp, "set_Speed", 1);
        g_GetJumpHeight = il2cpp_class_get_method_from_name(g_PlayerProp, "get_JumpHeight", 0);
        g_SetJumpHeight = il2cpp_class_get_method_from_name(g_PlayerProp, "set_JumpHeight", 1);
    }
    
    if (g_DropManager) {
        g_OneGPU = il2cpp_class_get_method_from_name(g_DropManager, "OneGPU", 0);
        g_StartGPU = il2cpp_class_get_method_from_name(g_DropManager, "StartGPU", 1);
    }
    
    if (g_OCBodyPart) {
        g_GetWeakTrans = il2cpp_class_get_method_from_name(g_OCBodyPart, "GetWeakTrans", 1);
        g_GetSpecialWeakTrans = il2cpp_class_get_method_from_name(g_OCBodyPart, "GetSpecialWeakTrans", 0);
        g_GetWeakTransByTag = il2cpp_class_get_method_from_name(g_OCBodyPart, "GetWeakTransByTag", 1);
    }
    
    if (Transform) {
        g_GetPosition = il2cpp_class_get_method_from_name(Transform, "get_position", 0);
        g_GetForward = il2cpp_class_get_method_from_name(Transform, "get_forward", 0);
    }
    
    auto Component = il2cpp_class_from_name(imgCore, "UnityEngine", "Component");
    if (Component) {
        g_GetTransform = il2cpp_class_get_method_from_name(Component, "get_transform", 0);
    }
    
    auto CameraManager = il2cpp_class_from_name(imgCSharp, "", "CameraManager");
    if (CameraManager) {
        g_GetMainCameraCom = il2cpp_class_get_method_from_name(CameraManager, "get_MainCameraCom", 0);
    }

    if (il2cpp_class_get_field_from_name && g_NewPlayerManager) {
        g_MainCtrlField = il2cpp_class_get_field_from_name(g_NewPlayerManager, "MainCtrl");
        g_PlayerDictField = il2cpp_class_get_field_from_name(g_NewPlayerManager, "PlayerDict");
    }

    // GMStateManager
    g_GMStateManager = il2cpp_class_from_name(imgCSharp, "", "GMStateManager");
    if (g_GMStateManager) {
        g_SetNoCostBullet = il2cpp_class_get_method_from_name(g_GMStateManager, "SetNoCostBullet", 1);
    }

    // Direct RVA functions
    g_GetPositionInjected = (GetPositionInjected_t)((BYTE*)g_GameAssembly + RVA_GET_POSITION_INJECTED);
    g_SetPositionInjected = (SetPositionInjected_t)((BYTE*)g_GameAssembly + RVA_SET_POSITION_INJECTED);
    g_GetWorldToCameraMatrix = (GetMatrix_t)((BYTE*)g_GameAssembly + RVA_CAMERA_WORLDTOCAMERAMATRIX);
    g_GetProjectionMatrix = (GetMatrix_t)((BYTE*)g_GameAssembly + RVA_CAMERA_PROJECTIONMATRIX);
    g_GetMainCameraComDirect = (GetMainCameraComDirect_t)((BYTE*)g_GameAssembly + RVA_GET_MAINCAMERACOM_DIRECT);
    g_GetWarCash = (GetWarCash_t)((BYTE*)g_GameAssembly + RVA_GET_WARCASH);
    g_SetWarCash = (SetWarCash_t)((BYTE*)g_GameAssembly + RVA_SET_WARCASH);
    g_GetPlayerPropFunc = (GetPlayerProp_t)((BYTE*)g_GameAssembly + RVA_GET_PLAYERPROP);
    g_GetFOV = (GetFOV_t)((BYTE*)g_GameAssembly + RVA_CAMERA_GET_FOV);
    g_SetFOV = (SetFOV_t)((BYTE*)g_GameAssembly + RVA_CAMERA_SET_FOV);

    // Weakness hit hack addresses
    g_CartoonDataSetSkilllRayAddr = (BYTE*)g_GameAssembly + RVA_CARTOONDATA_SETSKILLLRAY;
    g_CartoonDataPacketSkillRayAddr = (BYTE*)g_GameAssembly + RVA_CARTOONDATA_PACKETSKILLRAY;
    g_SClientHitInfoCtorAddr = (BYTE*)g_GameAssembly + RVA_SCLIENTHITINFO_CTOR;

    // Create weakness strings using il2cpp_string_new
    if (il2cpp_string_new) {
        g_WeaknessString = il2cpp_string_new("Monster_Weakness");
        g_SpecialWeaknessString = il2cpp_string_new("Monster_SpecialWeakness");
    }

    return g_GetMonsters && g_GetWeakTrans && g_GetPosition;
}

Il2CppObject* GetLocalPlayer() {
    if (!g_NewPlayerManager || !g_GetPlayer || !il2cpp_field_static_get_value || !g_MainCtrlField)
        return nullptr;
    
    int mainCtrl = 0;
    il2cpp_field_static_get_value(g_MainCtrlField, &mainCtrl);
    if (mainCtrl == 0) return nullptr;
    
    void* args[] = { &mainCtrl };
    return il2cpp_runtime_invoke(g_GetPlayer, nullptr, args, nullptr);
}

Il2CppObject* GetCurrentWeaponItemProp(Il2CppObject* localPlayer) {
    if (!localPlayer) return nullptr;
    
    auto playerCom = *(Il2CppObject**)((char*)localPlayer + OFFSET_PLAYERCOM);
    if (!playerCom) return nullptr;
    
    auto curWeaponSlot = *(Il2CppObject**)((char*)playerCom + OFFSET_CURWEAPONASSLOT);
    if (!curWeaponSlot) return nullptr;
    
    auto fstASObj = *(Il2CppObject**)((char*)curWeaponSlot + OFFSET_FSTASOBJ);
    if (!fstASObj) return nullptr;
    
    auto reloadCom = *(Il2CppObject**)((char*)fstASObj + OFFSET_RELOADCOM);
    if (!reloadCom) return nullptr;
    
    return *(Il2CppObject**)((char*)reloadCom + OFFSET_ITEMPROP);
}

void RefillAmmo(Il2CppObject* itemProp) {
    if (!itemProp || !g_GetMaxBullet || !g_GetCurBullet) return;

    auto maxBulletObj = il2cpp_runtime_invoke(g_GetMaxBullet, itemProp, nullptr, nullptr);
    if (!maxBulletObj) return;
    int maxBullet = *(int*)il2cpp_object_unbox(maxBulletObj);

    auto curBulletObj = il2cpp_runtime_invoke(g_GetCurBullet, itemProp, nullptr, nullptr);
    if (curBulletObj) {
        int curBullet = *(int*)il2cpp_object_unbox(curBulletObj);
        if (curBullet >= maxBullet) return;
    }

    void* args[] = { &maxBullet };
    if (g_SetCurBullet)
        il2cpp_runtime_invoke(g_SetCurBullet, itemProp, args, nullptr);
    if (g_SetClientCurBullet)
        il2cpp_runtime_invoke(g_SetClientCurBullet, itemProp, args, nullptr);
}

void SetPlayerSpeed(Il2CppObject* localPlayer, int speed) {
    if (!localPlayer || !g_SetSpeed) return;
    
    auto playerProp = *(Il2CppObject**)((char*)localPlayer + OFFSET_PLAYERPROP);
    if (!playerProp) return;
    
    void* args[] = { &speed };
    il2cpp_runtime_invoke(g_SetSpeed, playerProp, args, nullptr);
}

int GetPlayerSpeed(Il2CppObject* localPlayer) {
    if (!localPlayer || !g_GetSpeed) return 500;
    
    auto playerProp = *(Il2CppObject**)((char*)localPlayer + OFFSET_PLAYERPROP);
    if (!playerProp) return 500;
    
    auto result = il2cpp_runtime_invoke(g_GetSpeed, playerProp, nullptr, nullptr);
    if (result) return *(int*)il2cpp_object_unbox(result);
    return 500;
}

void SetJumpHeight(Il2CppObject* localPlayer, float height) {
    if (!localPlayer || !g_SetJumpHeight) return;
    
    auto playerProp = *(Il2CppObject**)((char*)localPlayer + OFFSET_PLAYERPROP);
    if (!playerProp) return;
    
    void* args[] = { &height };
    il2cpp_runtime_invoke(g_SetJumpHeight, playerProp, args, nullptr);
}

float GetJumpHeight(Il2CppObject* localPlayer) {
    if (!localPlayer || !g_GetJumpHeight) return 1.0f;
    
    auto playerProp = *(Il2CppObject**)((char*)localPlayer + OFFSET_PLAYERPROP);
    if (!playerProp) return 1.0f;
    
    auto result = il2cpp_runtime_invoke(g_GetJumpHeight, playerProp, nullptr, nullptr);
    if (result) return *(float*)il2cpp_object_unbox(result);
    return 1.0f;
}

void AutoPickup() {
    if (!g_PlayerDictField || !g_MainCtrlField || !g_GetPositionInjected || !g_SetPositionInjected)
        return;
    
    int mainCtrl = 0;
    il2cpp_field_static_get_value(g_MainCtrlField, &mainCtrl);
    if (mainCtrl == 0) return;
    
    Il2CppObject* playerDict = nullptr;
    il2cpp_field_static_get_value(g_PlayerDictField, &playerDict);
    if (!playerDict) return;
    
    auto entries = *(Il2CppObject**)((char*)playerDict + 0x18);
    int dictCount = *(int*)((char*)playerDict + 0x20);
    if (!entries || dictCount <= 0) return;
    
    Il2CppObject* playerTrans = nullptr;
    for (int i = 0; i < dictCount + 50; i++) {
        char* entryBase = (char*)entries + 0x20 + i * 24;
        int hashCode = *(int*)entryBase;
        if (hashCode < 0) continue;
        
        int key = *(int*)(entryBase + 0x8);
        if (key != mainCtrl) continue;
        
        auto playerObj = *(Il2CppObject**)(entryBase + 0x10);
        if (!playerObj) continue;
        
        playerTrans = *(Il2CppObject**)((char*)playerObj + OFFSET_GAMETRANS);
        break;
    }
    
    if (!playerTrans) return;
    
    Vector3 playerPos;
    g_GetPositionInjected(playerTrans, &playerPos);
    Vector3 targetPos = { playerPos.x, playerPos.y + 0.5f, playerPos.z };
    
    for (int i = 0; i < dictCount + 50; i++) {
        char* entryBase = (char*)entries + 0x20 + i * 24;
        int hashCode = *(int*)entryBase;
        if (hashCode < 0) continue;
        
        auto playerObj = *(Il2CppObject**)(entryBase + 0x10);
        if (!playerObj) continue;
        
        auto dropCom = *(Il2CppObject**)((char*)playerObj + OFFSET_DROPOPCOM);
        if (!dropCom) continue;
        
        auto gameTrans = *(Il2CppObject**)((char*)playerObj + OFFSET_GAMETRANS);
        if (!gameTrans) continue;
        
        g_SetPositionInjected(gameTrans, &targetPos);
    }
}


// Get best weakness transform (SpecialWeakness > Monster_Weakness)
void* GetBestWeaknessTrans(void* bodyPartCom) {
    if (!bodyPartCom) return nullptr;
    
    // 1순위: SpecialWeakness
    if (g_GetSpecialWeakTrans) {
        auto specialTrans = il2cpp_runtime_invoke(g_GetSpecialWeakTrans, bodyPartCom, nullptr, nullptr);
        if (specialTrans) return specialTrans;
    }
    
    // 2순위: Monster_Weakness (GetWeakTrans with findNearest=true)
    if (g_GetWeakTrans) {
        bool findNearest = true;
        void* args[] = { &findNearest };
        auto weakTrans = il2cpp_runtime_invoke(g_GetWeakTrans, bodyPartCom, args, nullptr);
        if (weakTrans) return weakTrans;
    }
    
    return nullptr;
}
