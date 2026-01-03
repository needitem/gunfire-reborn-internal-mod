#include "menu.h"
#include "dx11.h"
#include "../features/settings.h"
#include "../features/esp.h"
#include "imgui.h"
#include <cstdio>
#include <cmath>

void RenderMenu() {
    if (!g_ImGuiInitialized) return;
    
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("GFR Mod Menu [HOME to toggle]", &g_MenuVisible);
    
    ImGui::Text("=== Combat ===");
    ImGui::Checkbox("Silent Aim [F1]", &g_SilentAimEnabled);
    ImGui::Checkbox("Skill Aim [F7]", &g_SkillAimEnabled);
    ImGui::Checkbox("No Recoil [F4]", &g_NoRecoil);
    ImGui::Checkbox("No Spread [F5]", &g_NoSpread);
    ImGui::Checkbox("Fast Bullet [F6]", &g_FastBullet);
    if (g_FastBullet) {
        ImGui::SliderFloat("Bullet Speed", &g_BulletSpeedMultiplier, 1.0f, 200.0f);
    }
    ImGui::Checkbox("Big Radius [F9]", &g_BigRadius);
    if (g_BigRadius) {
        ImGui::SliderFloat("Radius", &g_RadiusValue, 1.0f, 999.0f);
    }
    
    ImGui::Separator();
    ImGui::Text("=== Player ===");
    ImGui::Checkbox("Infinite Ammo [F2]", &g_InfiniteAmmo);
    ImGui::Checkbox("Speed Boost [F3]", &g_SpeedBoost);
    if (g_SpeedBoost) {
        ImGui::SliderInt("Speed", &g_BoostedSpeed, 500, 2000);
    }
    ImGui::Checkbox("FOV Hack [F8]", &g_FOVEnabled);
    if (g_FOVEnabled) {
        ImGui::SliderFloat("FOV", &g_CustomFOV, 60.0f, 150.0f);
    }
    ImGui::Checkbox("Infinite Gold [F10]", &g_InfiniteGold);
    if (g_InfiniteGold) {
        ImGui::SliderInt("Gold Amount", &g_GoldAmount, 1000, 999999);
    }
    ImGui::Checkbox("Weakness Hit [F11]", &g_WeaknessHack);
    
    ImGui::Separator();
    ImGui::Text("=== ESP ===");
    ImGui::Text("Hold ALT to show ESP");
    ImGui::Text("Shows: Secret Rooms, Treasure Box");
    
    AcquireSRWLockShared(&g_ESPLock);
    ImGui::Text("Detected: %zu objects", g_ESPObjects.size());
    ReleaseSRWLockShared(&g_ESPLock);
    
    ImGui::Separator();
    ImGui::Text("=== Other ===");
    ImGui::Text("Mouse Wheel = Auto Pickup");
    ImGui::Text("END = Exit Mod");
    
    ImGui::End();
}

void RenderESPOverlay() {
    if (!g_ImGuiInitialized || !g_pRenderTargetView) return;
    
    RECT rect;
    GetClientRect(g_GameWindow, &rect);
    int screenWidth = rect.right - rect.left;
    int screenHeight = rect.bottom - rect.top;
    
    if (screenWidth <= 0 || screenHeight <= 0) return;
    
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)screenWidth, (float)screenHeight));
    ImGui::Begin("ESP Overlay", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    AcquireSRWLockShared(&g_ESPLock);
    for (const auto& obj : g_ESPObjects) {
        Vector3 screenPos;
        if (WorldToScreen(obj.worldPos, screenPos, screenWidth, screenHeight)) {
            if (screenPos.x >= -50 && screenPos.x <= screenWidth + 50 && 
                screenPos.y >= -50 && screenPos.y <= screenHeight + 50) {
                
                float boxSize = 30.0f / (obj.distance / 10.0f + 0.1f);
                boxSize = fmaxf(10.0f, fminf(50.0f, boxSize));
                
                ImU32 color = obj.isMonster ? IM_COL32(0, 128, 255, 255) : IM_COL32(0, 255, 0, 255);
                
                drawList->AddRect(
                    ImVec2(screenPos.x - boxSize, screenPos.y - boxSize),
                    ImVec2(screenPos.x + boxSize, screenPos.y + boxSize),
                    color, 0.0f, 0, 3.0f);
                
                char labelText[64];
                const char* label = obj.isMonster ? "Secret" : "Box";
                sprintf_s(labelText, "%s %.0fm", label, obj.distance);
                drawList->AddText(ImVec2(screenPos.x - 25, screenPos.y + boxSize + 2),
                    IM_COL32(255, 255, 255, 255), labelText);
            }
        }
    }
    ReleaseSRWLockShared(&g_ESPLock);
    
    ImGui::End();
    
    // Debug window
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(250, 100));
    ImGui::Begin("ESP Debug", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    AcquireSRWLockShared(&g_MatrixLock);
    bool matrixValid = g_ViewMatrixValid;
    ReleaseSRWLockShared(&g_MatrixLock);
    
    AcquireSRWLockShared(&g_ESPLock);
    size_t objCount = g_ESPObjects.size();
    ReleaseSRWLockShared(&g_ESPLock);
    
    ImGui::Text("ESP Active");
    ImGui::Text("Matrix: %s", matrixValid ? "OK" : "INVALID");
    ImGui::Text("Objects: %zu", objCount);
    
    ImGui::End();
}
